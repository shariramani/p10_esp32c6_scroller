
#include "rss_handler.h"
#include "p10_display.h"

void fetchAllRSSFeeds() {
  if (!hasInternet) {
    Serial.println("No internet connection - skipping RSS fetch");
    return;
  }
  
  logMemoryUsage("Before RSS fetch");
  Serial.println("Starting RSS feed fetch cycle...");
  
  clearRSSHeadlines(); // Clear old headlines
  
  int successCount = 0;
  for (const auto& feed : feeds) {
    if (feed.enabled) {
      handleFeedFetch(feed);
      successCount++;
      delay(500);
    }
  }
  
  Serial.printf("RSS fetch complete: %d feeds processed\n", successCount);
  logMemoryUsage("After RSS fetch");
}

void handleFeedFetch(const RSSFeed& feed) {
  Serial.printf("Fetching: %s\n", feed.name.c_str());

  HTTPClient http;
  http.setTimeout(HTTP_TIMEOUT);
  http.setUserAgent("ESP32-RSS-Scroller/1.0");

  if (!http.begin(feed.url)) {
    Serial.printf("%s - HTTP begin failed\n", feed.name.c_str());
    return;
  }

  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_MOVED_PERMANENTLY || httpCode == HTTP_CODE_FOUND) {
    String newUrl = http.getLocation();
    http.end();
    if (newUrl.length() > 0) {
      Serial.printf("%s - Redirected to: %s\n", feed.name.c_str(), newUrl.c_str());
      if (http.begin(newUrl)) {
        httpCode = http.GET();
      }
    }
  }

  if (httpCode <= 0) {
    Serial.printf("%s - HTTP error: %d\n", feed.name.c_str(), httpCode);
    http.end();
    return;
  }

  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("%s - HTTP status: %d\n", feed.name.c_str(), httpCode);
    http.end();
    return;
  }

  String payload = http.getString();
  http.end();
  
  Serial.printf("%s - Downloaded %d bytes\n", feed.name.c_str(), payload.length());

  if (payload.length() < 50) {
    Serial.printf("%s - Response too short\n", feed.name.c_str());
    return;
  }

  sanitizeXmlNamespaces(payload);

  tinyxml2::XMLDocument doc;
  if (doc.Parse(payload.c_str()) != tinyxml2::XML_SUCCESS) {
    Serial.printf("%s - XML parsing failed, using fallback\n", feed.name.c_str());
    extractTitlesFallback(payload, feed.name.c_str());
    return;
  }

  tinyxml2::XMLElement* root = doc.FirstChildElement("rss");
  if (!root) root = doc.FirstChildElement("feed");
  
  tinyxml2::XMLElement* channel = root;
  if (root && strcmp(root->Name(), "rss") == 0) {
    channel = root->FirstChildElement("channel");
  }

  if (!channel) {
    Serial.printf("%s - No channel/feed element found\n", feed.name.c_str());
    extractTitlesFallback(payload, feed.name.c_str());
    return;
  }

  int count = 0;
  const char* itemTag = strcmp(root->Name(), "feed") == 0 ? "entry" : "item";
  
  for (tinyxml2::XMLElement* item = channel->FirstChildElement(itemTag);
       item && count < settings.maxHeadlinesPerFeed;
       item = item->NextSiblingElement(itemTag)) {

    tinyxml2::XMLElement* titleElem = item->FirstChildElement("title");
    const char* titleText = titleElem ? titleElem->GetText() : nullptr;

    if (titleText) {
      String cleanTitle = sanitizeString(String(titleText));
      if (cleanTitle.length() > 5) {
        String headline = feed.name + ": " + cleanTitle;
        addRSSHeadline(headline);
        Serial.printf("%s #%d: %s\n", feed.name.c_str(), ++count, cleanTitle.c_str());
      }
    }
  }

  if (count == 0) {
    Serial.printf("%s - No valid headlines found\n", feed.name.c_str());
    extractTitlesFallback(payload, feed.name.c_str());
  }
}

bool isRecentNews(const char* pubDate) {
  if (!pubDate) return true; // Include if no date
  
  struct tm tm = {};
  // Try different date formats
  if (!strptime(pubDate, "%a, %d %b %Y %H:%M:%S", &tm)) {
    if (!strptime(pubDate, "%Y-%m-%dT%H:%M:%S", &tm)) {
      return true; // Include if can't parse
    }
  }
  
  time_t now = time(nullptr);
  time_t pubTime = mktime(&tm);
  double hoursDiff = difftime(now, pubTime) / 3600.0;
  
  return hoursDiff <= settings.maxNewsAgeHours;
}

void extractTitlesFallback(String& payload, const char* name) {
  int pos = 0, count = 0;
  
  while ((pos = payload.indexOf("<title>", pos)) != -1 && 
         count < settings.maxHeadlinesPerFeed) {
    int end = payload.indexOf("</title>", pos);
    if (end != -1) {
      String title = payload.substring(pos + 7, end);
      title = sanitizeString(title);
      
      if (title.length() > 5 && !title.startsWith("http")) {
        Serial.printf("%s [fallback #%d]: %s\n", name, ++count, title.c_str());
      }
      pos = end + 8;
    } else {
      break;
    }
  }
  
  if (count == 0) {
    Serial.printf("%s - No titles found in fallback\n", name);
  }
}

void sanitizeXmlNamespaces(String& xml) {
  xml.replace("media:", "");
  xml.replace("atom:", "");
  xml.replace("content:", "");
  xml.replace("dc:", "");
  xml.replace("feedburner:", "");
  xml.replace("slash:", "");
}
