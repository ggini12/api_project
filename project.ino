#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "";  // Wi-Fi SSID
const char* password = "";  // Wi-Fi 비밀번호
const char* apiKey = "";  // OpenWeatherMap API 키
const String city = "Gangwon-do,kr";  // 도시 이름과 국가 코드 (예: busan,kr)

const int ledPin = D2;  // LED 핀 번호

unsigned long previousMillis = 0;  // 이전 시간 저장
unsigned long blinkPreviousMillis = 0;  // LED 깜빡임 시간 저장
const unsigned long interval = 60000;  // 1분 (60,000 밀리초)
const unsigned long blinkInterval = 500;  // LED 깜빡임 간격 (500ms)

bool blinkState = false;  // LED 깜빡임 상태
bool isBlinking = false;  // LED 깜빡임 활성화 여부

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  // Wi-Fi 연결
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi!!!!!!!!!");
}

void loop() {
  unsigned long currentMillis = millis();

  // 1분 간격으로 데이터 가져오기
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    getWeatherData();
  }

  // 깜빡임 상태일 경우 LED 제어
  if (isBlinking) {
    handleBlinking(currentMillis);
  }
}

void getWeatherData() {
  WiFiClient wifiClient;
  HTTPClient http;

  String url = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "&appid=" + apiKey + "&units=metric";
  http.begin(wifiClient, url);

  int httpCode = http.GET();

  if (httpCode > 0) {
    String payload = http.getString();

    // 도시와 국가 정보가 제대로 반환되었는지 확인
    if (payload.indexOf("cod") != -1 && payload.indexOf("404") == -1) {  // 404 에러가 아니면 정상
      float rainPercent = parseRainPercent(payload);
      float temperature = parseTemperature(payload);

      // 도시 이름과 강수확률 출력
      Serial.print("도시: ");
      Serial.println(city);
      Serial.print("현재 온도: ");
      Serial.print(temperature);
      Serial.println("°C");
      Serial.print("강수확률: ");
      Serial.print(rainPercent);
      Serial.println("%");

      // 비가 오는지 여부 확인
      bool raining = isRaining(payload);
      if (raining) {
        Serial.println("비가 오고 있습니다.");
      } else {
        Serial.println("비가 오지 않고 있습니다.");
      }

      // 강수확률에 따른 LED 제어
      if (rainPercent >= 70.0) {
        isBlinking = false;  // 깜빡임 비활성화
        digitalWrite(ledPin, HIGH);  // 강수확률 70% 이상, LED 켜기
      } else if (rainPercent >= 50.0) {
        isBlinking = true;  // 깜빡임 활성화
      } else {
        isBlinking = false;  // 깜빡임 비활성화
        digitalWrite(ledPin, LOW);  // 강수확률 50% 미만, LED 끄기
      }
    } else {
      Serial.println("에러 : 나라이름이 다릅니다.");
    }
  } else {
    Serial.println("에러 : OpenWeatherMap과 연결되지 않았습니다.");
  }

  http.end();
}

float parseRainPercent(String json) {
  // JSON 파싱: OpenWeatherMap 응답에서 강수확률 데이터 추출
  int index = json.indexOf("pop");
  if (index != -1) {
    int start = json.indexOf(":", index) + 1;
    int end = json.indexOf(",", start);
    String rainPercent = json.substring(start, end);
    return rainPercent.toFloat() * 100;  // 확률을 퍼센트로 변환
  }
  return 0.0;  // 강수확률이 없으면 65% 반환
}

float parseTemperature(String json) {
  // JSON 파싱: OpenWeatherMap 응답에서 온도 데이터 추출
  int tempIndex = json.indexOf("\"temp\":");
  if (tempIndex != -1) {
    int start = tempIndex + 7;  // "temp": 이후 시작 위치
    int end = json.indexOf(",", start);
    String tempString = json.substring(start, end);
    return tempString.toFloat();  // 온도를 float로 변환
  }
  return 0.0;  // 온도가 없으면 0.0 반환
}

bool isRaining(String json) {
  // JSON 파싱: OpenWeatherMap 응답에서 weather 배열 확인
  int weatherIndex = json.indexOf("weather");
  if (weatherIndex != -1) {
    int mainIndex = json.indexOf("main", weatherIndex);
    if (mainIndex != -1) {
      int start = json.indexOf(":", mainIndex) + 1;
      int end = json.indexOf(",", start);
      String weatherMain = json.substring(start, end);
      if (weatherMain == "\"Rain\"") {
        return true;  // 비가 오는 경우
      }
    }
  }
  return false;  // 비가 오지 않는 경우
}

void handleBlinking(unsigned long currentMillis) {
  if (currentMillis - blinkPreviousMillis >= blinkInterval) {
    blinkPreviousMillis = currentMillis;
    blinkState = !blinkState;  // LED 상태 반전
    digitalWrite(ledPin, blinkState ? HIGH : LOW);
  }
}
