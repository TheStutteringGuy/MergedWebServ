# HTTP Error Codes Reference

## 4xx Client Errors (Request Problems)

**400 - Bad Request**
- The request cannot be processed
- Malformed request syntax, invalid request message framing, or deceptive request routing

**401 - Unauthorized** 
- Authentication is required
- Client must authenticate itself to get the requested response

**403 - Forbidden**
- Access to this resource is denied
- Client does not have access rights to the content (authenticated but not authorized)

**404 - Not Found**
- The requested resource was not found
- Server can't find the requested resource (most common error)

**405 - Method Not Allowed**
- Request method not supported
- The request method is known by the server but not supported by the target resource

**408 - Request Timeout**
- The request took too long
- Server would like to shut down this unused connection

**411 - Length Required**
- Content-Length header is required
- Server refuses to accept the request without a defined Content-Length

**414 - URI Too Long**
- The requested URI is too long
- URI provided was too long for the server to process

**415 - Unsupported Media Type**
- Media format is not supported
- Media format of the requested data is not supported by the server

**429 - Too Many Requests**
- Rate limit exceeded
- User has sent too many requests in a given amount of time

## 5xx Server Errors (Server Problems)

**500 - Internal Server Error**
- Something went wrong on our end
- Generic error message when server encounters an unexpected condition

**501 - Not Implemented**
- Server does not support this feature
- Request method is not supported by the server and cannot be handled

**502 - Bad Gateway**
- Invalid response from upstream server
- Server received an invalid response from the upstream server

**503 - Service Unavailable**
- Server is temporarily unavailable
- Server is not ready to handle the request (maintenance, overloaded, etc.)

**504 - Gateway Timeout**
- Upstream server did not respond
- Server acting as a gateway did not receive a timely response

**505 - HTTP Version Not Supported**
- HTTP version is not supported
- HTTP version used in the request is not supported by the server

## Quick Summary for Web Server Development

**Most Common Errors You'll Handle:**
- 400: Bad request format/syntax
- 401: Need login/authentication  
- 403: Logged in but no permission
- 404: File/resource doesn't exist
- 405: Wrong HTTP method (GET vs POST)
- 413: File upload too large
- 429: Rate limiting triggered
- 500: Your code crashed/exception
- 503: Server overloaded/maintenance mode
