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
    pub destinationLabel: String,
    //FIXME is in sub-structure: pub realtime_departure: Option<i64>,
    //FIXME is in sub-structure: pub scheduled_departure: i64,
    //TODO    pub difference: Option<i64>,
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
    println!("{}", &rawText);

    let body: std::result::Result<Station, serde_json::Error> = serde_json::from_str(&rawText);

    if body.is_err() {
        println!("Could not parse json {:?}", body.err());  
        return Option::None;
    }

    let cur_time = DateTime::<Utc>::default();

    // parse JSON result.. search of both directions
    let json = body.unwrap();
    for el in (json.graphQL.response.journeys.elements) {
        //TODO:
        /*let destination = (el.destination).to_string();
        let departure   = el.realtime_departure;
        let difference  = el.difference;

        if departure.is_some() {
            // get current time
            let time_s = departure.unwrap();
            let local_time = NaiveDateTime::from_timestamp_millis(time_s*1000).unwrap();
            let zoned_time : DateTime<Utc> = DateTime::from_utc(local_time, Utc);
            let europe_time = zoned_time.with_timezone(&Berlin);
            
            let hour = europe_time.hour();
            let minute = europe_time.minute();
            if zoned_time > cur_time {
                println!("------------- Future starts here ----------------");    
            }
            println!("{0} {1}:{2}", destination, hour, minute);
        }*/
    }

    Some("")
}
