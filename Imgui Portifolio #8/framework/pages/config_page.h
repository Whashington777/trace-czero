#pragma once



namespace config_page

{

    struct state

    {

        static constexpr int max_configs = 10;

        static constexpr int name_len    = 48;

        static constexpr int date_len    = 24;



        char names[max_configs][name_len] = { "Default", "Legit", "Rage" };

        char dates[max_configs][date_len] = { "2 days ago", "Yesterday", "Just now" };

        int  count     = 3;

        int  selected  = 0;

        char new_name[name_len] = {};

        char search[48]         = {};

        char status[96]         = "Ready";

        float status_alpha      = 1.f;

    };



    void draw(state& s, bool& blocks_window_drag);

}

