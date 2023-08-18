/* @file straba.rs
 * @brief fetch next depature of light rail vehicle
 */
use serde::Deserialize;
use serde_json::Value;

const STATION_URL:&str = "https://www.rnv-online.de/rest/departure/2494";

#[derive(Default, Debug, Clone, PartialEq, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct Station {
    pub id: i64,
    pub name: String,
    pub lines: Vec<Line>,
    pub journeys: Vec<Journey>,
}

#[derive(Default, Debug, Clone, PartialEq, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct Line {
    pub id: i64,
    pub text: String,
    pub icon_outlined: i64,
    pub icon_color: String,
    pub icon_text_color: i64,
}

#[derive(Default, Debug, Clone, PartialEq, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct Journey {
    pub line: Line,
    pub destination: String,
    pub barrier_level: String,
    pub loads_forecast_type: String,
    pub realtime_departure: i64,
    pub scheduled_departure: i64,
    pub difference: i64,
    pub canceled: bool,
}

pub struct NextDeparture {
    pub rheinau: i64,
    pub schoenau: i64,
}


pub fn fetch_data() -> Option<String> {
    let result = reqwest::blocking::get(STATION_URL);

    if result.is_err() {
        println!("Could not read station response {:?}", result.err());  
        return Option::None;
    }
    let text = result.unwrap().text();
    if text.is_err() {
        println!("Could not convert response {:?}", text.err());  
        return Option::None;
    }


    let body: std::result::Result<Value, serde_json::Error> = serde_json::from_str(&text.unwrap());

    if body.is_err() {
        println!("Could not parse json {:?}", body.err());  
        return Option::None;
    }

    let json = body.unwrap();
    let date = json["id"].to_string();
    Some(date)
}
