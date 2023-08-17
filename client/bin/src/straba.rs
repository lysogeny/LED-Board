/* @file straba.rs
 * @brief fetch next depature of light rail vehicle
 */
use std::error::Error;

const stationURL:&str = "https://www.rnv-online.de/rest/departure/2494/1";

pub fn fetchData() -> Result<(), Box<dyn Error>> {
    println!("Test RNV API");
    let resp = reqwest::blocking::get(stationURL)?.text()?;
    println!("{:#?}", resp);
    Ok(())
}
