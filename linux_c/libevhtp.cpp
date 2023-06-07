#include<iostream>
#include"evhtp.h"

using namespace std;

void testcb(evhtp_request_t *req, void *a) {
    evbuffer_add_reference(req->buffer_out, "openEuler\n", 10, NULL, NULL);
    evhtp_send_reply(req, EVHTP_RES_OK);
}


int main() {
    evbase_t *evbase = event_base_new();
    evhtp_t *htp = evhtp_new(evbase, NULL);

    // 设置回调函数
    evhtp_set_cb(htp, "/", testcb, NULL);
    // 监听本地的IP和8888端口
    evhtp_bind_socket(htp, "127.0.0.1", 8888, 1024);
    // 进入循环, 监听链接,http工作
    event_base_loop(evbase, 0);
    return 0;
}