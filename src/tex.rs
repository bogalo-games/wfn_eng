use vulkano::command_buffer::{AutoCommandBufferBuilder, CommandBuffer};
use vulkano::buffer::{CpuAccessibleBuffer, BufferUsage};
use vulkano::image::{StorageImage, Dimensions};
use vulkano::format::{ClearValue, Format};
use vulkano::device::{Device, Queue};
use vulkano::sync::GpuFuture;
use std::sync::Arc;
use image;

static NO_TEX_PATH: &'static str = "res/notex.png";

fn load_texture(device: Arc<Device>, queue: Arc<Queue>, path: &'static str) -> Arc<StorageImage<Format>> {
    let img = image::open(path).unwrap().to_rgba();
    let (width, height) = img.dimensions();
    let raw = img.into_raw();

    let mut buf: Arc<CpuAccessibleBuffer<[u8]>>;
    unsafe {
        let unsafe_buf: Arc<CpuAccessibleBuffer<[u8]>> = CpuAccessibleBuffer::uninitialized_array(
            device.clone(),
            raw.len(),
            BufferUsage::all()
        ).unwrap();

        let mut buffer_content = unsafe_buf.write().unwrap();
        let mut n = 0;
        for item in buffer_content.iter_mut() {
            *item = raw[n];
            n += 1;
        }

        buf = unsafe_buf.clone();
    }

    let image = StorageImage::new(
        device.clone(),
        Dimensions::Dim2d {
            width: width,
            height: height
        },
        Format::R8G8B8A8Unorm,
        Some(queue.family())
    ).unwrap();

    let command_buffer = AutoCommandBufferBuilder::primary_one_time_submit(device.clone(), queue.family()).unwrap()
        .clear_color_image(image.clone(), ClearValue::Float([0.0, 0.0, 0.0, 0.0])).unwrap()
        .copy_buffer_to_image(buf.clone(), image.clone()).unwrap()
        .build().unwrap();

    let finished = command_buffer.execute(queue.clone()).unwrap();
    finished.then_signal_fence_and_flush().unwrap()
        .wait(None).unwrap();

    return image;
}
