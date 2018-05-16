#[macro_use]
extern crate vulkano;

use vulkano::instance::InstanceExtensions;
use vulkano::instance::PhysicalDevice;
use vulkano::instance::Features;
use vulkano::instance::Instance;

use vulkano::device::DeviceExtensions;
use vulkano::device::Device;

use vulkano::buffer::CpuAccessibleBuffer;
use vulkano::buffer::BufferUsage;

use vulkano::command_buffer::AutoCommandBufferBuilder;
use vulkano::command_buffer::CommandBuffer;

use vulkano::sync::GpuFuture;

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
    let source_content = 0 .. 64;
    let dest_content = (0 .. 64).map(|_| 0);

    let source = CpuAccessibleBuffer::from_iter(
        device.clone(),
        BufferUsage::all(),
        source_content
    ).expect("Failed to create src");

    let dest = CpuAccessibleBuffer::from_iter(
        device.clone(),
        BufferUsage::all(),
        dest_content
    ).expect("Failed to create dst");

    let command_buffer = AutoCommandBufferBuilder::new(
        device.clone(),
        queue.family()
    ).unwrap()
        .copy_buffer(source.clone(), dest.clone()).unwrap()
        .build().expect("Could not create command buffer");

    let finished = command_buffer.execute(queue.clone()).unwrap();
    finished.then_signal_fence_and_flush().unwrap().wait(None).unwrap();

    let src_cont = source.read().unwrap();
    let dst_cont = dest.read().unwrap();

    assert_eq!(&*src_cont, &*dst_cont);
}
