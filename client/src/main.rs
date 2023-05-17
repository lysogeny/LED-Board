use std::net::UdpSocket;
use std::env;

const IMAGE_WIDTH_BYTE : usize = (5*32) / 8; /* one byte contains 8 lets, one in each bit */
const IMAGE_HEIGHT_BYTE : usize = 40;

fn send_package(ipaddress: String) {
    let mut image = [0; (IMAGE_WIDTH_BYTE*IMAGE_HEIGHT_BYTE)+1];
    // Brightnes
    image[0] = 0x80;
    // row 0, led 0
    image[1] = 1;
    // row 0, led 8
    image[2] = 1;
    // row 0, led 16 and 17
    image[3] = 3;
    // row 0, led 24, 25, 26
    image[4] = 7;
    let socket = UdpSocket::bind("0.0.0.0:4242").expect("couldn't bind to address");
    socket.send_to(&image, ipaddress + ":4242").expect("couldn't send data");
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
