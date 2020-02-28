
#include <iostream>
#include <banner/banner.hpp>

#include "tri_renderer.hpp";

using namespace ban;

int
main()
{
    platform* pl = new platform("Hello my dog");
    graphics* gr = new graphics(pl);
    renderer* re = new renderer(&gr->get_swapchain());

    tri_renderer* tr = new tri_renderer(re);

    pl->on_update.connect([&]() {
        re->pre_render();
        tr->update();
        re->render();
    });

    pl->start_loop();
    re->wait();
}
