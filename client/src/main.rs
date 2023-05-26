use bit::BitIndex;
use core::time;
use embedded_graphics::{
    image::{Image, ImageRaw},
    mono_font::{iso_8859_1::FONT_6X10, MonoTextStyle},
    pixelcolor::{BinaryColor, Rgb565},
    prelude::*,
    primitives::{Line, PrimitiveStyle},
    text::Text,
};
use openweathermap::{self, CurrentWeather};
use std::net::UdpSocket;
use std::{env, sync::mpsc::Receiver, thread};

const IMAGE_SIZE_BYTE: usize = (IMAGE_WIDTH_BYTE * IMAGE_HEIGHT) as usize; /* one byte contains 8 LEDs, one in each bit */
const IMAGE_WIDTH: u32 = 5 * 32;
const IMAGE_WIDTH_BYTE: u32 = IMAGE_WIDTH / 8; /* one byte contains 8 LEDs, one in each bit */

const IMAGE_HEIGHT: u32 = 40;
const IMAGE_HEIGHT_BYTE: u32 = 40;
const IMAGE_LENGTH: usize = (IMAGE_WIDTH_BYTE * IMAGE_HEIGHT_BYTE) as usize;
const PACKAGE_LENGTH: usize = (IMAGE_LENGTH + 1) as usize;

struct UdpDisplay {
    image: [u8; IMAGE_SIZE_BYTE],
}

impl OriginDimensions for UdpDisplay {
    fn size(&self) -> Size {
        Size::new(IMAGE_WIDTH, IMAGE_HEIGHT)
    }
}

impl DrawTarget for UdpDisplay {
    type Color = BinaryColor;
    type Error = core::convert::Infallible;

    fn fill_contiguous<I>(
        &mut self,
        area: &embedded_graphics::primitives::Rectangle,
        colors: I,
    ) -> Result<(), Self::Error>
    where
        I: IntoIterator<Item = Self::Color>,
    {
        self.draw_iter(
            area.points()
                .zip(colors)
                .map(|(pos, color)| Pixel(pos, color)),
        )
    }

    fn fill_solid(
        &mut self,
        area: &embedded_graphics::primitives::Rectangle,
        color: Self::Color,
    ) -> Result<(), Self::Error> {
        self.fill_contiguous(area, core::iter::repeat(color))
    }

    fn clear(&mut self, color: Self::Color) -> Result<(), Self::Error> {
        self.fill_solid(&self.bounding_box(), color)
    }

    fn draw_iter<I>(&mut self, pixels: I) -> Result<(), Self::Error>
    where
        I: IntoIterator<Item = Pixel<Self::Color>>,
    {
        for pixel in pixels {
            if pixel.0.x < 0 {
                continue;
            }
            if pixel.0.y < 0 {
                continue;
            }

            let x = pixel.0.x as u32;
            let y = pixel.0.y as u32;
            if x > (IMAGE_WIDTH - 1) {
                continue;
            }
            if y > (IMAGE_HEIGHT - 1) {
                continue;
            }

            let y = y as u32;
            let v = pixel.1.is_on();
            //println!("pint {x} {y} is on {v}");
            let offset: usize = (x + y * IMAGE_WIDTH) as usize;

            let subbit: usize = (offset % 8).into();
            let byte_offset: usize = (offset / 8).into();

            let current = &mut self.image[byte_offset];
            current.set_bit(subbit, v);
        }
        return Ok(());
    }
}

fn send_package(ipaddress: String, data: &Option<Result<CurrentWeather, String>>) {
    let mut package: [u8; PACKAGE_LENGTH] = [0; PACKAGE_LENGTH];

    // Brightness
    package[0] = 128;

    let mut display = UdpDisplay {
        image: [0; IMAGE_SIZE_BYTE],
    };

    let style = PrimitiveStyle::with_stroke(BinaryColor::On, 1);
    let text_style = MonoTextStyle::new(&FONT_6X10, BinaryColor::On);

    Line::new(Point::new(0, 0), Point::new((IMAGE_WIDTH - 1) as i32, 0))
        .into_styled(style)
        .draw(&mut display)
        .unwrap();

    
    match data {
        Some(v) => match v {
            Err(error) => {
                Text::new(&error, Point::new(0, 5), text_style)
                    .draw(&mut display)
                    .unwrap();
                println!("{}", &error);
            }
            Ok(result) => {
                let mut y = 10;
                for condition in result.weather.as_slice() {

                    let text = condition.main.to_owned() + " " +  &condition.description + " " + &condition.icon;
                    Text::new(&text, Point::new(0, y), text_style)
                        .draw(&mut display)
                        .unwrap();
                    println!("{}", &condition.main);
                    y += 10;
                }
            }
        },
        None => {
            Text::new("Waiting for data", Point::new(20, 30), text_style)
                .draw(&mut display)
                .unwrap();
            println!("{}", "no result");
        }
    }

    package[1..PACKAGE_LENGTH].copy_from_slice(&display.image);

    let socket = UdpSocket::bind("0.0.0.0:4242").expect("couldn't bind to address");
    socket
        .send_to(&package, ipaddress + ":4242")
        .expect("couldn't send data");
    println!("Packet sent");
}

fn help() {
    println!(
        "usage:
LEDboardClient <ip address>"
    );
    println!("one argument necessary!");
    println!("<ip address>");
}

fn main() {
    let args: Vec<String> = env::args().collect();

    match args.len() {
        // no arguments passed
        1 => {
            // show a help message
            help();
        }
        // one argument passed
        2 => {
            let ip = &args[1];
            let receiver = openweathermap::init(
                "Mannheim",
                "metric",
                "de",
                "978882ab9dd05e7122ff2b0aef2d3e55",
                60,
            );

            let mut lastData = Option::None;

            loop {
                let delay = time::Duration::from_millis(10000);
                thread::sleep(delay);
                let answer = openweathermap::update(&receiver);

                match answer {
                    Some(_) => {
                        lastData = answer;
                    }
                    None => {

                    }
                }

                send_package(ip.to_string(), &lastData);
            }
        }
        // all the other cases
        _ => {
            // show a help message
            help();
        }
    }
}
