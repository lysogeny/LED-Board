use chrono::{Utc, DateTime, NaiveDateTime, Timelike};
use chrono_tz::Europe::Berlin;
/* @file straba.rs
 * @brief fetch next depature of light rail vehicle
 */
use serde_json::Value;
use serde::Deserialize;

const STATION_URL:&str = "https://www.rnv-online.de/rest/departure/2494";

/* ******************** JSON Description ****************************** */
#[derive(Default, Debug, Clone, PartialEq, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct Station {
    pub id: String,
    pub name: String,
    pub graphQL: GraphQL,
}

#[derive(Default, Debug, Clone, PartialEq, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct GraphQL {
    pub response: GraphQLResponse,
}

#[derive(Default, Debug, Clone, PartialEq, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct GraphQLResponse {
    pub name: String,
    pub journeys: JourneysElement,
}

#[derive(Default, Debug, Clone, PartialEq, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct JourneysElement {
    pub elements: Vec<Journey>,
}

#[derive(Default, Debug, Clone, PartialEq, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct Line {
    pub lineGroup: LineGroup,
}

#[derive(Default, Debug, Clone, PartialEq, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct LineGroup {
    pub label: String,
}

#[derive(Default, Debug, Clone, PartialEq, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct Journey {
    pub line: Line,
    pub canceled: bool,
    pub stops: Vec<StopsElement>,
}

#[derive(Default, Debug, Clone, PartialEq, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct StopsElement {
    pub destinationLabel:   String,
    pub plannedDeparture:   IsoStringDateTime,
    pub realtimeDeparture:  IsoStringDateTime,
}

#[derive(Default, Debug, Clone, PartialEq, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct IsoStringDateTime {
    pub isoString: Option<String>,
}

// Return value
pub struct NextDeparture {
    pub rheinau: i64,
    pub schoenau: i64,
}

pub fn fetch_data() -> Option<&'static str> {
    let result = reqwest::blocking::get(STATION_URL);
    println!("Start Straba Crawler");

    if result.is_err() {
        println!("Could not read station response {:?}", result.err());  
        return Option::None;
    }
    let text = result.unwrap().text();
    if text.is_err() {
        println!("Could not convert response {:?}", text.err());
        return Option::None;
    }
    let rawText = &text.unwrap();

    let body: std::result::Result<Station, serde_json::Error> = serde_json::from_str(&rawText);

    if body.is_err() {
        println!("Could not parse json {:?}", body.err());  
        println!("------------------------- %< ----------------------------");
        println!("{}", &rawText);
        println!("------------------------- %< ----------------------------");
        return Option::None;
    }

    let cur_time = DateTime::<Utc>::default();

    // parse JSON result.. search of both directions
    let json = body.unwrap();
    for el in (json.graphQL.response.journeys.elements) {
        println!("Line {:}", el.line.lineGroup.label);  
        for stop in el.stops {
            if stop.realtimeDeparture.isoString.is_some() {
                println!("To      {:} {:?}", stop.destinationLabel, stop.realtimeDeparture.isoString)
            } else {
                println!("Planned {:} {:?}", stop.destinationLabel, stop.plannedDeparture.isoString)
            }
        }
    }

    Some("")
}
