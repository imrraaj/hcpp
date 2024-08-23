#include "hcpp/hcpp.h"

int main()
{
    Server app;
    app.get("/", [](Request req, Response res)
            { res.send("Hello World!"); });

    app.get("/a", [](Request req, Response res)
            { 
                sleep(5);
                res.send("Hello A!"); });

    app.post("/b", [](Request req, Response res)
             {
                // print the body of the request
                std::cout << req.body << std::endl;
                 res.send(req.body); });

    app.listen(3000, []()
               { std::cout << "Server is running on port 3000" << std::endl; });
}