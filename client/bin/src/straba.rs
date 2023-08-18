/* @file straba.rs
 * @brief fetch next depature of light rail vehicle
 */
use std::error::Error;
use serde::Deserialize;

const stationURL:&str = "https://www.rnv-online.de/rest/departure/2494";

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
    pub iconOutlined: i64,
    pub iconColor: String,
    pub iconTextColor: i64,
}

#[derive(Default, Debug, Clone, PartialEq, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct Journey {
    pub line: Line,
    pub destination: String,
    pub barrierLevel: String,
    pub loadsForecastType: String,
    pub realtimeDeparture: i64,
    pub scheduledDeparture: i64,
    pub difference: i64,
    pub canceled: bool,
}

pub struct NextDeparture {
    pub rheinau: i64,
    pub schoenau: i64,
}


pub fn fetchData() -> Result<String, reqwest::Error> {
    let result = reqwest::blocking::get(stationURL);

    let response = match result {
        Ok(res) => res,
        Err(err) => return reqwest::Error(err),
    };

    let body = serde_json::from_str(&text);

    let json = match body {
        Ok(json) => json,
        Err(err) => return reqwest::Error(err),
    };
    let date = json["id"].to_string();
    Ok(date)
}
