use std::net::UdpSocket;
use std::env;
use embedded_graphics::{
    image::{Image, ImageRaw},
    pixelcolor::{BinaryColor, Rgb565},
    prelude::*, primitives::{PrimitiveStyle, Line},
};


const IMAGE_SIZE_BYTE : usize = (IMAGE_WIDTH_BYTE * IMAGE_HEIGHT) as usize; /* one byte contains 8 LEDs, one in each bit */
const IMAGE_WIDTH : u32 = 5*32;
const IMAGE_WIDTH_BYTE : u32 = IMAGE_WIDTH / 8; /* one byte contains 8 LEDs, one in each bit */

const IMAGE_HEIGHT : u32 = 40;
const IMAGE_HEIGHT_BYTE : u32 = 40;
const IMAGE_LENGTH : usize = (IMAGE_WIDTH_BYTE * IMAGE_HEIGHT_BYTE) as usize;
const PACKAGE_LENGTH : usize = (IMAGE_LENGTH + 1) as usize;


struct UdpDisplay {
    imageSlice: [u8; IMAGE_SIZE_BYTE]
}

impl OriginDimensions for UdpDisplay {
    fn size(&self) -> Size {
        Size::new(IMAGE_WIDTH, IMAGE_HEIGHT)
    }
}

impl DrawTarget for UdpDisplay {
    type Color = BinaryColor;
    type Error = core::convert::Infallible;

    fn fill_contiguous<I>(&mut self, area: &embedded_graphics::primitives::Rectangle, colors: I) -> Result<(), Self::Error>
    where
        I: IntoIterator<Item = Self::Color>,
    {
        self.draw_iter(
            area.points()
                .zip(colors)
                .map(|(pos, color)| Pixel(pos, color)),
        )
    }

    fn fill_solid(&mut self, area: &embedded_graphics::primitives::Rectangle, color: Self::Color) -> Result<(), Self::Error> {
        self.fill_contiguous(area, core::iter::repeat(color))
    }

    fn clear(&mut self, color: Self::Color) -> Result<(), Self::Error> {
        self.fill_solid(&self.bounding_box(), color)
    }

    fn draw_iter<I>(&mut self, pixels: I) -> Result<(), Self::Error>
    where
        I: IntoIterator<Item = Pixel<Self::Color>> {
        todo!()
    }

}


fn send_package(ipaddress: String) {
    let mut package = [0; PACKAGE_LENGTH];
    package[0] = 128;
    let imageSlice: &mut[u8;IMAGE_SIZE_BYTE] = &mut package[1..PACKAGE_LENGTH].try_into().unwrap();


    let mut display = UdpDisplay {
        imageSlice
    };


    const IMAGE: &[u8] = &[0 as u8; ((IMAGE_WIDTH_BYTE * IMAGE_HEIGHT_BYTE) as usize)];
    let raw_image = ImageRaw::<BinaryColor>::new(IMAGE, IMAGE_WIDTH);

    Line::new(Point::new(50, 20), Point::new(60, 35))
    .into_styled(PrimitiveStyle::with_stroke(BinaryColor::On, 1))
    .draw(&mut raw_image)?;

    // Brightness
    
    imageSlice.copy_from_slice(&IMAGE);


    // TODO convert RgbImage into image buffer:
    // let pixel = img[(100, 100)];::

    let socket = UdpSocket::bind("0.0.0.0:4242").expect("couldn't bind to address");
    socket.send_to(&package, ipaddress + ":4242").expect("couldn't send data");
    println!("Packet sent");
}

fn help() {
    println!("usage:
LEDboardClient <ip address>");
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
        },
        // one argument passed
        2 => {
            let ip = &args[1];
            send_package(ip.to_string());
        },
        // all the other cases
        _ => {
            // show a help message
            help();
        }
    }
}
