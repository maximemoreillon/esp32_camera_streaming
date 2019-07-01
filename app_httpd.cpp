// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "esp_http_server.h"
#include "esp_timer.h"
#include "esp_camera.h"
#include "img_converters.h"
#include "Arduino.h"
#include "credentials.h";

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t server = NULL;

typedef struct {
        httpd_req_t *req;
        size_t len;
} jpg_chunking_t;

static size_t jpg_encode_stream(void * arg, size_t index, const void* data, size_t len){
    jpg_chunking_t *j = (jpg_chunking_t *)arg;
    if(!index){
        j->len = 0;
    }
    if(httpd_resp_send_chunk(j->req, (const char *)data, len) != ESP_OK){
        return 0;
    }
    j->len += len;
    return len;
}

static esp_err_t stream_handler(httpd_req_t *req){
  
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];

  // response headers
  httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*"); // Allowing CORS

  // Streaming loop
  while(true){

    // Frame capture
    fb = esp_camera_fb_get();
    
    if (!fb) {
      // Could not capture a frame
      Serial.println(F("[CAMERA] Camera capture failed"));
      res = ESP_FAIL;
    }
    else {
      // Get the frame buffer and its length
      _jpg_buf_len = fb->len;
      _jpg_buf = fb->buf;
    }

    // If no error until now, send response to client
    if(res == ESP_OK){
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    
    // Clearing up memory
    if(fb){
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if(_jpg_buf){
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if(res != ESP_OK){
      break;
    }
  }
  
  // Return if loop is broken
  return res;
}

static esp_err_t frame_handler(httpd_req_t *req){
  
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;

    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println(F("[CAMERA] Camera capture failed"));
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }

    // headers
    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*"); // CORS header
    res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
    
    esp_camera_fb_return(fb);

    return res;
}

void startCameraServer(){
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_uri_t index_uri = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = stream_handler,
        .user_ctx  = NULL
    };

    
    httpd_uri_t frame_uri = {
        .uri       = "/frame",
        .method    = HTTP_GET,
        .handler   = frame_handler,
        .user_ctx  = NULL
    };
    

    
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &index_uri);
        httpd_register_uri_handler(server, &frame_uri);
    }

    Serial.printf("[HTTP] Started web server on port: '%d'\n", config.server_port);
}
