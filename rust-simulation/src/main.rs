use log;
use env_logger;
use std::{thread, time::Duration};
use std::net::UdpSocket;
use embedded_graphics::{
    pixelcolor::BinaryColor,
    mono_font::{ascii::FONT_10X20, MonoTextStyle},
    text::Text,
    image::{Image, ImageRaw},
    prelude::*,
};
use embedded_graphics_simulator::{
    BinaryColorTheme, OutputSettingsBuilder, SimulatorDisplay, SimulatorEvent, Window,
};

const IMAGE_SIZE_BYTE: usize = (IMAGE_WIDTH_BYTE * IMAGE_HEIGHT) as usize; // one byte contains 8 LEDs, one in each bit
const PANEL_WIDTH: u32 = 32 ;
const PANEL_WIDTH_BYTE: u32 = PANEL_WIDTH / 8; // one byte contains 8 LEDs, one in each bit
const PANEL_COUNT: u32 = 5 ;
const PANEL_PIXELS: u32 = PANEL_WIDTH * 40;
const PANEL_BYTES: u32 = PANEL_PIXELS / 8; // one byte contains 8 LEDs, one in each bit
const IMAGE_WIDTH: u32 = PANEL_COUNT * PANEL_WIDTH;
const IMAGE_WIDTH_BYTE: u32 = IMAGE_WIDTH / 8; // one byte contains 8 LEDs, one in each bit
const IMAGE_HEIGHT: u32 = 40;
const IMAGE_HEIGHT_BYTE: u32 = 40;
const IMAGE_LENGTH: usize = (IMAGE_WIDTH_BYTE * IMAGE_HEIGHT_BYTE) as usize;
const PACKAGE_LENGTH: usize = (IMAGE_LENGTH + 1) as usize;

struct Display {
    window: Window,
    display: SimulatorDisplay<BinaryColor>,
}

impl Display {
    fn new() -> Display {
        let output_settings = OutputSettingsBuilder::new()
            .theme(BinaryColorTheme::OledBlue)
            .scale(4)
            .build();
        Display{
            display: SimulatorDisplay::new(Size::new(32*5, 40)),
            window: Window::new("LED Board", &output_settings),
        }
    }
    fn clear(&mut self) {
        self.display.clear(BinaryColor::Off).unwrap();
    }
    fn update(&mut self) {
        self.window.update(&self.display);
    }
    fn has_quit(&mut self) -> bool {
        self.window.events().any(|e| e == SimulatorEvent::Quit)
    }
}

struct Client {
    addr: String, 
    port: i32,
    socket: UdpSocket,
    display: Display
}

type ImgData = [u8; IMAGE_LENGTH];

impl Client {
    fn new(addr: &str, port: i32) -> Client {
        let addr_full = format!("{addr}:{port}");
        log::info!("Listening on {addr_full}");
        let socket = UdpSocket::bind(addr_full).expect("Couldn't bind");
        Client{
            addr: addr.to_string(), 
            port,
            socket,
            display: Display::new()
        }
    }
    fn recv_image(&mut self) -> ImgData {
        let mut data = [0; PACKAGE_LENGTH];
        // Read packages until one is PACKAGE_LENGTH
        loop {
            match self.socket.recv_from(&mut data) {
                Err(error) => panic!("{:}", error),
                Ok((PACKAGE_LENGTH, src_addr)) => {
                    log::debug!("Received image from {}", src_addr);
                    let data = &mut data[1..];
                    for i in 0..(IMAGE_LENGTH as usize) {
                        data[i] = data[i].reverse_bits();
                    }
                    return data.try_into().unwrap();
                },
                Ok((bytes_read, src_addr)) => {
                    log::info!("Responding to status checking message from {}", src_addr);
                    let data = &mut data[..bytes_read];
                    match self.socket.send_to(&data, &src_addr) {
                        Ok(_) => {},
                        Err(e) => log::error!("{:}", e),
                    };
                }
            }
        }
    }
    fn run(&mut self) -> Result<(), ()> {
        self.display.clear();
        self.display.update();
        'running: loop {
            self.display.clear();
            let data = self.recv_image();
            let image_raw = ImageRaw::<BinaryColor>::new(&data, IMAGE_WIDTH);
            let image = Image::new(&image_raw, Point::zero());
                image.draw(&mut self.display.display).unwrap();
            self.display.update();
            if self.display.has_quit() {
                break 'running Ok(());
            }
        }
    }
}

fn main() {
    env_logger::init();
    let mut client = Client::new("0.0.0.0", 4242);
    client.run().unwrap();
    /*
    let mut simulator = Simulator::new();
    match simulator.run() {
        Ok(v) => println!("Graceful exit {}", v), 
        Err(_) => eprintln!("Failed!"),
    }
    */
}
