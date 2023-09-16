use chrono::DateTime;
use std::time::{SystemTime, UNIX_EPOCH};
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
    pub request_time: i64,
    pub outbound_station: String,
    pub outbound_diff: i64,
    pub inbound_station: String,
    pub inbound_diff: i64,
    pub failure: bool,
}

pub fn fetch_data() -> NextDeparture {

    let st_now = SystemTime::now();
    let seconds = st_now.duration_since(UNIX_EPOCH).unwrap().as_secs();
    let url = &format!("{}?datetime={}", STATION_URL, seconds);
    let result = reqwest::blocking::get(url);
    
    let mut returnValue = NextDeparture {
        failure  : false,
        outbound_station  : String::from(""),
        outbound_diff : 10000,
        inbound_station : String::from(""),
        inbound_diff : 10000,
        request_time : seconds as i64,
    };

    println!("Start Straba Crawler");

    if result.is_err() {
        println!("Could not read station response {:?}", result.err());
        returnValue.failure = true;
        return returnValue;
    }
    let text = result.unwrap().text();
    if text.is_err() {
        println!("Could not convert response {:?}", text.err());
        returnValue.failure = true;
        return returnValue;
    }
    let rawText = &text.unwrap();

    let body: std::result::Result<Station, serde_json::Error> = serde_json::from_str(&rawText);

    if body.is_err() {
        println!("Could not parse json {:?}", body.err());  
        println!("------------------------- %< ----------------------------");
        println!("{}", &rawText);
        println!("------------------------- %< ----------------------------");
        returnValue.failure = true;
        return returnValue;
    }

    // parse JSON result.. search of both directions
    let json = body.unwrap();
    for el in (json.graphQL.response.journeys.elements) {
        println!("Line {:}", el.line.lineGroup.label);  
        for stop in el.stops {
            // use only valid data
            if stop.realtimeDeparture.isoString.is_some() && 
                stop.destinationLabel != "" {
                let txt_departure = stop.realtimeDeparture.isoString.unwrap();
                let next_departure = DateTime::parse_from_rfc3339(&txt_departure).unwrap();
                
                let diff = next_departure.timestamp() - (seconds  as i64);
                println!("To      {:} {:} (in {:} seconds)", stop.destinationLabel, txt_departure, diff );
                
                if stop.destinationLabel.contains("Rheinau") {
                    if (diff <  returnValue.outbound_diff) {
                        returnValue.outbound_station = stop.destinationLabel;
                        returnValue.outbound_diff = diff;
                    }
                } else if stop.destinationLabel.contains("Hochschule") ||
                            stop.destinationLabel.contains("Hauptbahnhof") ||
                            stop.destinationLabel.contains("Schönau") {
                    if (diff <  returnValue.inbound_diff) {
                        returnValue.inbound_station = stop.destinationLabel;
                        returnValue.inbound_diff = diff;
                    }
                }
            } else {
                println!("Planned {:} {:?}", stop.destinationLabel, stop.plannedDeparture.isoString)
            }
        }
    }

    returnValue
}
