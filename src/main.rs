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
}
