#[macro_use]
extern crate vulkano;
#[macro_use]
extern crate vulkano_shader_derive;
extern crate vulkano_win;
extern crate winit;

extern crate image;

use vulkano::instance::PhysicalDevice;
use vulkano::instance::Instance;

use vulkano::device::DeviceExtensions;
use vulkano::device::Device;

use vulkano::command_buffer::AutoCommandBufferBuilder;
use vulkano::command_buffer::DynamicState;

use vulkano::buffer::CpuAccessibleBuffer;
use vulkano::buffer::BufferUsage;

use vulkano::sync::GpuFuture;
use vulkano::sync::now;

use vulkano::pipeline::viewport::Viewport;
use vulkano::pipeline::GraphicsPipeline;

use vulkano::framebuffer::Framebuffer;
use vulkano::framebuffer::Subpass;

use vulkano::swapchain::SwapchainCreationError;
use vulkano::swapchain::SurfaceTransform;
use vulkano::swapchain::AcquireError;
use vulkano::swapchain::PresentMode;
use vulkano::swapchain::Swapchain;
use vulkano::swapchain;


use vulkano_win::VkSurfaceBuild;


use winit::WindowBuilder;
use winit::EventsLoop;


use std::sync::Arc;
use std::mem;


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


/// Getting a window working, and hopefully getting everything to work together!
fn windowing(physical: PhysicalDevice, instance: Arc<Instance>) {
    let mut events_loop = EventsLoop::new();
    let window = WindowBuilder::new().build_vk_surface(
        &events_loop,
        instance.clone()
    ).expect("Failed to create window");

    let queue_family = physical.queue_families()
        .find(|&q| q.supports_graphics() && window.is_supported(q).unwrap_or(false))
        .expect("Could not find a graphical queue.");

    let (device, mut queues) = {
        let extensions = DeviceExtensions {
            khr_swapchain: true,
            .. DeviceExtensions::none()
        };

        Device::new(
            physical,
            physical.supported_features(),
            &extensions,
            [ (queue_family, 0.5) ].iter().cloned()
        ).expect("Failed to create a device")
    };

    let queue = queues.next().unwrap();

    let mut dimensions = {
        let (width, height) = window.window().get_inner_size().unwrap();
        [width, height]
    };

    let (mut swapchain, mut images) = {
        let caps = window.capabilities(physical)
            .expect("Failde to get surface capabilities");

        let alpha = caps.supported_composite_alpha.iter().next().unwrap();
        dimensions = caps.current_extent.unwrap_or(dimensions);

        let format = caps.supported_formats[0].0;

        Swapchain::new(
            device.clone(),
            window.clone(),
            caps.min_image_count,
            format,
            dimensions,
            1,
            caps.supported_usage_flags,
            &queue,
            SurfaceTransform::Identity,
            alpha,
            PresentMode::Fifo,
            true,
            None
        ).expect("Failed to create swap chain")
    };

    let vertex_buffer = CpuAccessibleBuffer::from_iter(
        device.clone(),
        BufferUsage::all(),
        [
            Vertex { position: [  -0.5, -0.25 ] },
            Vertex { position: [   0.0,  0.5  ] },
            Vertex { position: [  0.25, -0.1  ] },
        ].iter().cloned()
    ).expect("Failed to create buffer");

    let vert = vert::Shader::load(device.clone()).unwrap();
    let frag = frag::Shader::load(device.clone()).unwrap();

    let render_pass = Arc::new(
        single_pass_renderpass!(
            device.clone(),
            attachments: {
                color: {
                    load: Clear,
                    store: Store,
                    format: swapchain.format(),
                    samples: 1,
                }
            },

            pass: {
                color: [color],
                depth_stencil: {}
            }
        ).unwrap()
    );

    let pipeline = Arc::new(
        GraphicsPipeline::start()
            .vertex_input_single_buffer::<Vertex>()
            .vertex_shader(vert.main_entry_point(), ())
            .triangle_list()
            .viewports_dynamic_scissors_irrelevant(1)
            .fragment_shader(frag.main_entry_point(), ())
            .render_pass(Subpass::from(render_pass.clone(), 0).unwrap())
            .build(device.clone())
            .unwrap()
    );

    let mut framebuffers: Option<Vec<Arc<Framebuffer<_, _>>>> = None;


    let mut recreate_swapchain = false;
    let mut previous_frame_end = Box::new(now(device.clone())) as Box<GpuFuture>;

    loop {
        previous_frame_end.cleanup_finished();

        if recreate_swapchain {
            dimensions = {
                let (new_width, new_height) = window.window().get_inner_size().unwrap();
                [new_width, new_height]
            };

            let (new_swapchain, new_images) = match swapchain.recreate_with_dimension(dimensions) {
                Ok(r) => r,

                Err(SwapchainCreationError::UnsupportedDimensions) => {
                    println!("Unsupported dimensions {:?}", dimensions);
                    recreate_swapchain = false;
                    continue;
                },

                Err(err) => panic!("{:?}", err)
            };

            mem::replace(&mut swapchain, new_swapchain);
            mem::replace(&mut images, new_images);

            framebuffers = None;
            recreate_swapchain = false;
        }

        if framebuffers.is_none() {
            let new_framebuffers = Some(
                images.iter().map(|image| {
                    Arc::new(
                        Framebuffer::start(render_pass.clone())
                            .add(image.clone()).unwrap()
                            .build().unwrap()
                    )
                }).collect::<Vec<_>>()
            );

            mem::replace(&mut framebuffers, new_framebuffers);
        }

        let (image_num, acquire_future) = match swapchain::acquire_next_image(swapchain.clone(), None) {
            Ok(r) => r,

            Err(AcquireError::OutOfDate) => {
                println!("Swapchain out of date.");
                recreate_swapchain = true;
                continue;
            },

            Err(err) => panic!("{:?}", err)
        };

        let command_buffer = AutoCommandBufferBuilder::primary_one_time_submit(device.clone(), queue.family()).unwrap()
            .begin_render_pass(framebuffers.as_ref().unwrap()[image_num].clone(), false, vec![[0.1, 0.1, 0.1, 1.0].into()]).unwrap()
            .draw(
                pipeline.clone(),
                DynamicState {
                    line_width: None,
                    viewports: Some(vec![Viewport {
                        origin: [0.0, 0.0],
                        dimensions: [dimensions[0] as f32, dimensions[1] as f32],
                        depth_range: 0.0 .. 1.0
                    }]),
                    scissors: None,
                },
                vertex_buffer.clone(), (), ()
            ).unwrap()
            .end_render_pass().unwrap()
            .build().unwrap();

        let future = previous_frame_end.join(acquire_future)
            .then_execute(queue.clone(), command_buffer).unwrap()
            .then_swapchain_present(queue.clone(), swapchain.clone(), image_num)
            .then_signal_fence_and_flush().unwrap();
        previous_frame_end = Box::new(future) as Box<_>;

        let mut done = false;
        events_loop.poll_events(|event| {
            match event {
                winit::Event::WindowEvent { event: winit::WindowEvent::Closed, .. } => {
                    done = true;
                },

                winit::Event::WindowEvent { event: winit::WindowEvent::Resized(_, _), .. } => {
                    recreate_swapchain = true;
                },

                _ => ()
            }
        });

        if done { return; }
    }
}

/// Entry point to the application.
fn main() {
    let instance = {
        let extensions = vulkano_win::required_extensions();
        Instance::new(
            None,
            &extensions,
            None
        ).expect("Failed to create instance")
    };

    let physical = PhysicalDevice::enumerate(&instance).next().expect("No device available");

    windowing(
        physical.clone(),
        instance.clone()
    )
}
