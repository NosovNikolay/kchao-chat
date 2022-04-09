#pragma once

#define RESPONSE_TEMPLATE "HTTP/1.1 %s\r\n%s\r\n\r\n%s"

// RESPONSE STATUS

#define RESPONSE_100 "100 Continue"
#define RESPONSE_Continue RESPONSE_100

#define RESPONSE_101 "101 Switching Protocol"
#define RESPONSE_Switching_Protocol RESPONSE_101

#define RESPONSE_102 "102 Processing"
#define RESPONSE_Processing RESPONSE_102

#define RESPONSE_103 "103 Early Hints"
#define RESPONSE_Early_Hints RESPONSE_103

#define RESPONSE_200 "200 OK"
#define RESPONSE_OK RESPONSE_200

#define RESPONSE_201 "201 Created"
#define RESPONSE_Created RESPONSE_201

#define RESPONSE_202 "202 Accepted"
#define RESPONSE_Accepted RESPONSE_202

#define RESPONSE_203 "203 Non-Authoritative Information"
#define RESPONSE_Non_Authoritative_Information RESPONSE_203

#define RESPONSE_204 "204 No Content"
#define RESPONSE_No_Content RESPONSE_204

#define RESPONSE_205 "205 Reset Content"
#define RESPONSE_Reset_Content RESPONSE_205

#define RESPONSE_206 "206 Partial Content"
#define RESPONSE_Partial_Content RESPONSE_206

#define RESPONSE_300 "300 Multiple Choice"
#define RESPONSE_Multiple_Choice RESPONSE_300

#define RESPONSE_301 "301 Moved Permanently"
#define RESPONSE_Moved_Permanently RESPONSE_301

#define RESPONSE_302 "302 Found"
#define RESPONSE_Found RESPONSE_302

#define RESPONSE_303 "303 See Other"
#define RESPONSE_See_Other RESPONSE_303

#define RESPONSE_304 "304 Not Modified"
#define RESPONSE_Not_Modified RESPONSE_304

#define RESPONSE_305 "305 Use Proxy"
#define RESPONSE_Use_Proxy RESPONSE_305

#define RESPONSE_306 "306 Switch Proxy"
#define RESPONSE_Switch_Proxy RESPONSE_306

#define RESPONSE_307 "307 Temporary Redirect"
#define RESPONSE_Temporary_Redirect RESPONSE_307

#define RESPONSE_308 "308 Permanent Redirect"
#define RESPONSE_Permanent_Redirect RESPONSE_308

#define RESPONSE_400 "400 Bad Request"
#define RESPONSE_Bad_Request RESPONSE_400

#define RESPONSE_401 "401 Unauthorized"
#define RESPONSE_Unauthorized RESPONSE_401

#define RESPONSE_402 "402 Payment Required"
#define RESPONSE_Payment_Required RESPONSE_402

#define RESPONSE_403 "403 Forbidden"
#define RESPONSE_Forbidden RESPONSE_403

#define RESPONSE_404 "404 Not Found"
#define RESPONSE_Not_Found RESPONSE_404

#define RESPONSE_405 "405 Method Not Allowed"
#define RESPONSE_Method_Not_Allowed RESPONSE_405

#define RESPONSE_406 "406 Not Acceptable"
#define RESPONSE_Not_Acceptable RESPONSE_406

#define RESPONSE_407 "407 Proxy Authentication Required"
#define RESPONSE_Proxy_Authentication_Required RESPONSE_407

#define RESPONSE_408 "408 Request Timeout"
#define RESPONSE_Request_Timeout RESPONSE_408

#define RESPONSE_409 "409 Conflict"
#define RESPONSE_Conflict RESPONSE_409

#define RESPONSE_410 "410 Gone"
#define RESPONSE_Gone RESPONSE_410

#define RESPONSE_411 "411 Length Required"
#define RESPONSE_Length_Required RESPONSE_411

#define RESPONSE_412 "412 Precondition Failed"
#define RESPONSE_Precondition_Failed RESPONSE_412

#define RESPONSE_413 "413 Request Entity Too Large"
#define RESPONSE_Request_Entity Too_Large RESPONSE_413

#define RESPONSE_414 "414 Request-URI Too Long"
#define RESPONSE_Request_URI_Too_Long RESPONSE_414

#define RESPONSE_415 "415 Unsupported Media Type"
#define RESPONSE_Unsupported_Media_Type RESPONSE_415

#define RESPONSE_416 "416 Requested Range Not Satisfiable"
#define RESPONSE_Requested_Range_Not_Satisfiable RESPONSE_416

#define RESPONSE_417 "417 Expectation Failed"
#define RESPONSE_Expectation_Failed RESPONSE_417

#define RESPONSE_500 "500 Internal Server Error"
#define RESPONSE_Internal_Server_Error RESPONSE_500

#define RESPONSE_501 "501 Not Implemented"
#define RESPONSE_Not_Implemented RESPONSE_501

#define RESPONSE_502 "502 Bad Gateway"
#define RESPONSE_Bad_Gateway RESPONSE_502

#define RESPONSE_503 "503 Service Unavailable"
#define RESPONSE_Service_Unavailable RESPONSE_503

#define RESPONSE_504 "504 Gateway Timeout"
#define RESPONSE_Gateway_Timeout RESPONSE_504

#define RESPONSE_505 "505 HTTP Version Not Supported"
#define RESPONSE_HTTP_Version_Not_Supported RESPONSE_505

// HEADERS

#define HEADER_CONTENT_LENGTH "Content-Length"
#define HEADER_CONTENT_TYPE "Content-Type"
#define HEADER_AUTHORIZATION "Authorization"

// CONTENT TYPE

#define CONTENT_TYPE_application_EDI_X12 "application/EDI-X12"
#define CONTENT_TYPE_application_EDIFACT "application/EDIFACT"
#define CONTENT_TYPE_application_ogg "application/ogg"
#define CONTENT_TYPE_application_pdf "application/pdf"
#define CONTENT_TYPE_application_json "application/json"
#define CONTENT_TYPE_application_xml "application/xml"
#define CONTENT_TYPE_application_zip "application/zip"

#define CONTENT_TYPE_audio_mpeg "audio/mpeg"
#define CONTENT_TYPE_audio_x_wav "audio/x-wav"

#define CONTENT_TYPE_image_gif "image/gif"
#define CONTENT_TYPE_image_jpeg "image/jpeg"
#define CONTENT_TYPE_image_png "image/png"
#define CONTENT_TYPE_image_tiff "image/tiff"
#define CONTENT_TYPE_image_x_icon "image/x-icon"

#define CONTENT_TYPE_multipart_mixed "multipart/mixed"
#define CONTENT_TYPE_multipart_alternative "multipart/alternative"
#define CONTENT_TYPE_multipart_related "multipart/related"
#define CONTENT_TYPE_multipart_form_data "multipart/form-data"

#define CONTENT_TYPE_text_css "text/css"
#define CONTENT_TYPE_text_csv "text/csv"
#define CONTENT_TYPE_text_html "text/html"
#define CONTENT_TYPE_text_javascript "text/javascript"
#define CONTENT_TYPE_text_plain "text/plain"
#define CONTENT_TYPE_text_xml "text/xml"

#define CONTENT_TYPE_video_mpeg "video/mpeg"
#define CONTENT_TYPE_video_mp4 "video/mp4"
#define CONTENT_TYPE_video_webm "video/webm"

// REQUEST TYPE

#define REQUEST_ALL "*"
#define REQUEST_GET "GET"
#define REQUEST_HEAD "HEAD"
#define REQUEST_POST "POST"
#define REQUEST_PUT "PUT"
#define REQUEST_DELETE "DELETE"
#define REQUEST_CONNECT "CONNECT"
#define REQUEST_OPTIONS "OPTIONS"
#define REQUEST_TRACE "TRACE"
#define REQUEST_PATCH "PATCH"
