use serde::Deserialize;

#[derive(Default, Debug, Clone, PartialEq, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct Forecast {
    pub city: City,
    pub cod: String,
    pub message: f64,
    pub cnt: f64,
    pub list: Vec<List>,
}

#[derive(Default, Debug, Clone, PartialEq, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct City {
    pub id: f64,
    pub name: String,
    pub coord: Coord,
    pub country: String,
    pub population: f64,
    pub timezone: f64,
    pub sunrise: i64,
    pub sunset: i64,
}

#[derive(Default, Debug, Clone, PartialEq, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct Coord {
    pub lon: f64,
    pub lat: f64,
}

#[derive(Default, Debug, Clone, PartialEq, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct List {
    pub dt: i64,
    pub main: Main,
    pub weather: Vec<Weather>,
    pub wind: Wind,
    pub clouds: Clouds,
    pub pop: f64,
    pub rain: Option<Rain>,
}

#[derive(Default, Debug, Clone, PartialEq, Deserialize)]
pub struct Wind{
    pub speed:f64,
    pub deg:f64,
    pub gust:f64
}

#[derive(Default, Debug, Clone, PartialEq, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct Rain{
    #[serde(rename = "3h")]
    pub three_hours:f64
}

#[derive(Default, Debug, Clone, PartialEq, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct Clouds{
    all:f64
}

#[derive(Default, Debug, Clone, PartialEq, Deserialize)]
pub struct Main {
    pub temp: f64,
    pub feels_like: f64,
    pub temp_min: f64,
    pub temp_max: f64,
    pub pressure: f64,
    pub sea_level: f64,
    pub grnd_level: f64,
    pub humidity: f64,
    pub temp_kf: f64,
}

#[derive(Default, Debug, Clone, PartialEq, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct FeelsLike {
    pub day: f64,
    pub night: f64,
    pub eve: f64,
    pub morn: f64,
}

#[derive(Default, Debug, Clone, PartialEq, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct Weather {
    pub id: f64,
    pub main: String,
    pub description: String,
    pub icon: String,
}
