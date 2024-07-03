#include "server/webserver.h"

int main(){
    webserver server(
        1316, 3, 0, true,
        3306, "test_user", "test_password", 
        "test_db", 12, 6, true, 1024
    );
    server.start();
}