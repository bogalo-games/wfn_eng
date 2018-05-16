#[macro_use]
extern crate vulkano;

#[macro_use]
extern crate vulkano_shader_derive;

extern crate image;

use vulkano::instance::InstanceExtensions;
use vulkano::instance::PhysicalDevice;
use vulkano::instance::Features;
use vulkano::instance::Instance;

use vulkano::device::DeviceExtensions;
use vulkano::device::Device;
use vulkano::device::Queue;

use vulkano::command_buffer::AutoCommandBufferBuilder;
use vulkano::command_buffer::CommandBuffer;
use vulkano::command_buffer::DynamicState;

use vulkano::buffer::CpuAccessibleBuffer;
use vulkano::buffer::BufferUsage;

use vulkano::image::StorageImage;
use vulkano::image::Dimensions;

use vulkano::format::Format;

use vulkano::sync::GpuFuture;

use vulkano::pipeline::viewport::Viewport;
use vulkano::pipeline::GraphicsPipeline;
use vulkano::pipeline::ComputePipeline;

use vulkano::framebuffer::Framebuffer;
use vulkano::framebuffer::Subpass;

use vulkano::descriptor::descriptor_set::PersistentDescriptorSet;


use image::ImageBuffer;
use image::Rgba;


use std::sync::Arc;


mod cs {
    #[derive(VulkanoShader)]
    #[ty = "compute"]
    #[src = "
#version 450

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(set = 0, binding = 0, rgba8) uniform writeonly image2D img;

void main() {
    vec2 norm_coordinates = (gl_GlobalInvocationID.xy + vec2(0.5)) / vec2(imageSize(img));
    vec2 c = (norm_coordinates - vec2(0.5)) * 2.0 - vec2(1.0, 0.0);

    vec2 z = vec2(0.0, 0.0);
    float i;
    for (i = 0.0; i < 1.0; i += 0.005) {
        z = vec2(
            z.x * z.x - z.y * z.y + c.x,
            z.y * z.x + z.x * z.y + c.y
        );

        if (length(z) > 4.0) {
            break;
        }
    }

    vec4 to_write = vec4(vec3(i), 1.0);
    imageStore(img, ivec2(gl_GlobalInvocationID.xy), to_write);
}"]

    struct _Dummy;
}

/// Creates a mandelbrot fractal via a compute shader, and then outputs it to
/// image.png.
fn make_mandelbrot(device: Arc<Device>, queue: Arc<Queue>, dim: u32) {
    let image = StorageImage::new(
        device.clone(),
        Dimensions::Dim2d { width: dim, height: dim },
        Format::R8G8B8A8Unorm,
        Some(queue.family())
    ).unwrap();

    let shader = cs::Shader::load(device.clone()).expect("Failde to create shader");

    let compute_pipeline = Arc::new(
        ComputePipeline::new(
            device.clone(),
            &shader.main_entry_point(),
            &()
        ).expect("Failed to create compute pipeline")
    );

    let set = Arc::new(
        PersistentDescriptorSet::start(
            compute_pipeline.clone(),
            0
        ).add_image(image.clone()).unwrap()
        .build().unwrap()
    );

    let buf = CpuAccessibleBuffer::from_iter(
        device.clone(),
        BufferUsage::all(),
        (0 .. dim * dim * 4).map(|_| 0u8)
    ).unwrap();

    let command_buffer = AutoCommandBufferBuilder::new(
        device.clone(),
        queue.family()
    ).unwrap()
        .dispatch([ dim / 8, dim / 8, 1 ], compute_pipeline.clone(), set.clone(), ()).unwrap()
        .copy_image_to_buffer(image.clone(), buf.clone()).unwrap()
        .build().unwrap();

    let finished = command_buffer.execute(queue.clone()).unwrap();
    finished.then_signal_fence_and_flush().unwrap().wait(None).unwrap();

    let buffer_content = buf.read().unwrap();
    let image = ImageBuffer::<Rgba<u8>, _>::from_raw(dim, dim, &buffer_content[..]).unwrap();
    image.save("image.png").unwrap();

}

/// Vertex shader for the graphics demo.
mod vert {
    #[derive(VulkanoShader)]
    #[ty = "vertex"]
    #[src = "
#version 450

layout(location = 0) in vec2 position;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
}"]

    struct _Dummy;
}

/// Fragment shader for the graphics demo.
mod frag {
    #[derive(VulkanoShader)]
    #[ty = "fragment"]
    #[src = "
#version 450

layout(location = 0) out vec4 f_color;

void main() {
    f_color = vec4(1.0, 0.0, 0.0, 1.0);
}
"]

    struct _Dummy;
}

/// A vertex type, used for the graphics pipeline demo.
#[derive(Copy, Clone)]
struct Vertex {
    position: [f32; 2]
}
impl_vertex!(Vertex, position);

/// The graphics pipeline demo. We're going to render a triangle!
fn graphics_pipeline(device: Arc<Device>, queue: Arc<Queue>, dim: u32) {
    let image = StorageImage::new(
        device.clone(),
        Dimensions::Dim2d { width: dim, height: dim },
        Format::R8G8B8A8Unorm,
        Some(queue.family())
    ).unwrap();

    let vertices = vec![
        Vertex { position: [ -0.5,  -0.5 ] },
        Vertex { position: [  0.0,   0.5 ] },
        Vertex { position: [  0.5, -0.25 ] }
    ];

    let vertex_buffer = CpuAccessibleBuffer::from_iter(
        device.clone(),
        BufferUsage::all(),
        vertices.into_iter()
    ).unwrap();

    let vert_shader = vert::Shader::load(device.clone()).expect("Failed to create vertex shader");
    let frag_shader = frag::Shader::load(device.clone()).expect("Failed to create fragment shader");

    let render_pass = Arc::new(
        single_pass_renderpass!(
            device.clone(),
            attachments: {
                color: {
                    load: Clear,
                    store: Store,
                    format: Format::R8G8B8A8Unorm,
                    samples: 1,
                }
            },

            pass: {
                color: [color],
                depth_stencil: {}
            }
        ).unwrap()
    );

    let framebuffer = Arc::new(
        Framebuffer::start(render_pass.clone())
            .add(image.clone()).unwrap()
            .build().unwrap()
    );

    let pipeline = Arc::new(
        GraphicsPipeline::start()
            .vertex_input_single_buffer::<Vertex>()
            .vertex_shader(vert_shader.main_entry_point(), ())
            .viewports_dynamic_scissors_irrelevant(1)
            .fragment_shader(frag_shader.main_entry_point(), ())
            .render_pass(Subpass::from(render_pass.clone(), 0).unwrap())
            .build(device.clone())
            .unwrap()
    );

    let dynamic_state = DynamicState {
        viewports: Some(vec![Viewport {
            origin: [0.0, 0.0],
            dimensions: [ dim as f32, dim as f32 ],
            depth_range: 0.0 .. 1.0
        }]),

        .. DynamicState::none()
    };

    let buf = CpuAccessibleBuffer::from_iter(
        device.clone(),
        BufferUsage::all(),
        (0 .. dim * dim * 4).map(|_| 0u8)
    ).unwrap();


    let command_buffer = AutoCommandBufferBuilder::primary_one_time_submit(
        device.clone(),
        queue.family()
    ).unwrap()
        .begin_render_pass(framebuffer.clone(), false, vec![[0.1, 0.1, 0.1, 1.0].into()]).unwrap()
        .draw(pipeline.clone(), dynamic_state, vertex_buffer.clone(), (), ()).unwrap()
        .end_render_pass().unwrap()
        
        .copy_image_to_buffer(image.clone(), buf.clone()).unwrap()
        .build().unwrap();

    let finished = command_buffer.execute(queue.clone()).unwrap();
    finished.then_signal_fence_and_flush().unwrap().wait(None).unwrap();

    let buffer_content = buf.read().unwrap();
    let image = ImageBuffer::<Rgba<u8>, _>::from_raw(dim, dim, &buffer_content[..]).unwrap();
    image.save("triangle.png").unwrap();
}

/// Entry point to the application.
fn main() {
    //
    // Setting up the context.
    //
    let instance = Instance::new(
        None,
        &InstanceExtensions::none(),
        None
    ).expect("Failed to create instance");

    let physical = PhysicalDevice::enumerate(&instance).next().expect("No device available");

    let queue_family = physical.queue_families()
        .find(|&q| q.supports_graphics())
        .expect("Could not find a graphical queue.");

    let (device, mut queues) = Device::new(
        physical,
        &Features::none(),
        &DeviceExtensions::none(),
        [ (queue_family, 0.5) ].iter().cloned()
    ).expect("Failed to create a device");

    let queue = queues.next().unwrap();

    //
    // Messing around
    //
    make_mandelbrot(device.clone(), queue.clone(), 512);
    graphics_pipeline(device.clone(), queue.clone(), 512);
}
