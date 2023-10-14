use chrono::DateTime;
use std::time::{SystemTime, UNIX_EPOCH};
use log;

use serde::Deserialize;

const STATION_URL:&str = "https://www.rnv-online.de/rest/departure/2494";

/* ******************** JSON Description ****************************** */
#[derive(Default, Debug, Clone, PartialEq, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct Station {
    pub id: String,
    pub name: String,
    #[serde(alias = "graphQL")]
    pub graph_ql: GraphQL,
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
    pub line_group: LineGroup,
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
    pub destination_label:   String,
    pub planned_departure:   IsoStringDateTime,
    pub realtime_departure:  IsoStringDateTime,
}

#[derive(Default, Debug, Clone, PartialEq, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct IsoStringDateTime {
    pub iso_string: Option<String>,
}

#[derive(Debug)]
enum APIError {
    Request(reqwest::Error),
    Read(reqwest::Error),
    Parse(serde_json::Error),
}

fn api_request(url: &String) -> Result<Station, APIError> {
    let body = reqwest::blocking::get(url).map_err(APIError::Request)?;
    let text = body.text().map_err(APIError::Read)?;
    let result = serde_json::from_str(&text).map_err(APIError::Parse);
    result
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

impl NextDeparture {
    fn new(seconds: i64) -> NextDeparture {
        NextDeparture {
            failure  : false,
            outbound_station  : String::from("Error"),
            outbound_diff : 666*60,
            inbound_station : String::from("Error"),
            inbound_diff : 666*60,
            request_time : seconds,
        }
    }
    fn populate_departure(&mut self, result: Station) {
        let journeys = result.graph_ql.response.journeys.elements;
        log::debug!("Found {} journeys", journeys.len());
        for el in journeys {
            for stop in el.stops {
                // use only valid data
                if stop.realtime_departure.iso_string.is_some() && 
                    stop.destination_label != "" {
                    let txt_departure = stop.realtime_departure.iso_string.unwrap();
                    let next_departure = DateTime::parse_from_rfc3339(&txt_departure).unwrap();
                    
                    let diff = next_departure.timestamp() - (self.request_time  as i64);
                    log::debug!("To      {:} {:} (in {:} seconds)", stop.destination_label, txt_departure, diff );
                    
                    if stop.destination_label.contains("Rheinau") {
                        if diff < self.outbound_diff {
                            self.outbound_station = stop.destination_label;
                            self.outbound_diff = diff;
                        }
                    } else if stop.destination_label.contains("Hochschule") ||
                                stop.destination_label.contains("Hauptbahnhof") ||
                                stop.destination_label.contains("Schönau") {
                        if diff < self.inbound_diff {
                            self.inbound_station = stop.destination_label;
                            self.inbound_diff = diff;
                        }
                    }
                } else {
                    log::debug!("Planned {:} {:?}", stop.destination_label, stop.planned_departure.iso_string)
                }
            }
        }
    }
}

pub fn fetch_data() -> NextDeparture {
    let st_now = SystemTime::now();
    let seconds = st_now.duration_since(UNIX_EPOCH).unwrap().as_secs();
    let url = &format!("{}?datetime={}", STATION_URL, seconds);
    let mut return_value = NextDeparture::new(seconds as i64);
    match api_request(url) {
        Ok(content) => return_value.populate_departure(content),
        Err(error) => log::error!("Error! {:?}", error)
    };
    return return_value
}
