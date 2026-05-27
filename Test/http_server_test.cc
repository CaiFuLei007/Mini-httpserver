
#include "http_server.h"
#include <iostream>
#include <sstream>

std::string ToJson(const std::string& key, const std::string& value)
{
    return "{\"" + key + "\": \"" + value + "\"}";
}

int main()
{
    HttpServer server(8080);

    server.SetBaseDir("/home/banju/Mini-moduo/web");
    // server.SetThreadCount(3);
    // server.SetSelfRelease(30);

    // GET /hello
    server.Get("/hello", [](std::shared_ptr<HttpRequest> req, std::shared_ptr<HttpResponse> resp) {
        resp->SetResponseCode(200);
        resp->SetBody(ToJson("message", "Hello, World!"), "application/json");
    });

    // GET /user?name=xxx
    server.Get("/user", [](std::shared_ptr<HttpRequest> req, std::shared_ptr<HttpResponse> resp) {
        std::string name = req->GetParam("name");
        if (name.empty()) name = "anonymous";
        resp->SetResponseCode(200);
        resp->SetBody(ToJson("user", name), "application/json");
    });

    // POST /login  body: username=xxx&password=xxx
    server.Post("/login", [](std::shared_ptr<HttpRequest> req, std::shared_ptr<HttpResponse> resp) {
        std::string body = req->GetBody();
        resp->SetResponseCode(200);
        std::ostringstream oss;
        oss << "{\"status\": \"ok\", \"body\": \"" << body << "\"}";
        resp->SetBody(oss.str(), "application/json");
    });

    // POST /echo
    server.Post("/echo", [](std::shared_ptr<HttpRequest> req, std::shared_ptr<HttpResponse> resp) {
        resp->SetResponseCode(200);
        resp->SetBody(req->GetBody(), "text/plain");
    });

    // PUT /update
    server.Put("/update", [](std::shared_ptr<HttpRequest> req, std::shared_ptr<HttpResponse> resp) {
        std::string body = req->GetBody();
        resp->SetResponseCode(200);
        std::ostringstream oss;
        oss << "{\"status\": \"updated\", \"data\": \"" << body << "\"}";
        resp->SetBody(oss.str(), "application/json");
    });

    // DELETE /remove
    server.Delete("/remove", [](std::shared_ptr<HttpRequest> req, std::shared_ptr<HttpResponse> resp) {
        resp->SetResponseCode(200);
        resp->SetBody(ToJson("status", "deleted"), "application/json");
    });

    std::cout << "========================================" << std::endl;
    std::cout << "  HttpServer listening on port 8080"     << std::endl;
    std::cout << "  BaseDir     : ./web"                   << std::endl;
    std::cout << "  ThreadCount : 3"                       << std::endl;
    std::cout << "  SelfRelease : 30s"                     << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "  Routes:"                               << std::endl;
    std::cout << "    GET    /hello       -> Hello World"   << std::endl;
    std::cout << "    GET    /user?name=x -> echo user"    << std::endl;
    std::cout << "    POST   /login       -> login test"   << std::endl;
    std::cout << "    POST   /echo        -> echo body"    << std::endl;
    std::cout << "    PUT    /update      -> update test"  << std::endl;
    std::cout << "    DELETE /remove      -> delete test"  << std::endl;
    std::cout << "    GET    /            -> web/index.html (static)" << std::endl;
    std::cout << "    GET    /404.html    -> web/404.html  (static)" << std::endl;
    std::cout << "    *      /xxx         -> 404 error"    << std::endl;
    std::cout << "========================================" << std::endl;

    server.Run();
    return 0;
}
