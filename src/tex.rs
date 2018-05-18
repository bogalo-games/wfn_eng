use vulkano::command_buffer::{AutoCommandBufferBuilder, CommandBuffer};
use vulkano::buffer::{CpuAccessibleBuffer, BufferUsage};
use vulkano::image::{StorageImage, Dimensions};
use vulkano::format::{ClearValue, Format};
use vulkano::device::{Device, Queue};
use vulkano::sync::GpuFuture;
use std::sync::Arc;
use image;

pub static NO_TEX_PATH: &'static str = "res/notex.png";

/// Given a Device and a Queue, produce a StorageImage filled with the contents
/// of an image on disk.
pub fn load_texture(device: Arc<Device>, queue: Arc<Queue>, path: &'static str) -> Arc<StorageImage<Format>> {
    let img = image::open(path).unwrap().to_rgba();
    let (width, height) = img.dimensions();
    let raw = img.into_raw();

    let buf = CpuAccessibleBuffer::from_iter(
        device.clone(),
        BufferUsage::all(),
        raw.iter().cloned()
    ).unwrap();

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
