#ifdef VS_HACK
    // This is not included on compilation, just in VS Code
    // to make IntelliSense work
    #include "PCHClient.h"
#endif

#include "Screen.h"

#include "../../utils/StrManip.h"

namespace Screen {
    Gtk::Window* window;
    Gtk::ScrolledWindow* scroll;
    Gtk::Box* chat;
    Glib::RefPtr<Gtk::TextBuffer> input_buffer;

    Client* client;
}

void auto_scroll(Gtk::Allocation& alocator) {
    Gtk::Adjustment* scroll_adjustment = Screen::scroll->get_vadjustment().get();
    scroll_adjustment->set_value(
		scroll_adjustment->get_upper() - scroll_adjustment->get_page_size());
}

void send_button_click() {
    std::string str = Screen::input_buffer->get_text();

    std::replace(str.begin(), str.end(), '\n', ' ');
    std::replace(str.begin(), str.end(), '\r', ' ');

	str = trim(str);

    Screen::client->parse_command(std::ref(str));

	Screen::input_buffer->set_text("");
}

void Screen::setup(Client* cli) {
    client = cli;

    window = new Gtk::Window;

    window->set_default_size(600, 400);
    window->set_title("Chat");
    window->set_position(Gtk::WIN_POS_CENTER);

    Gtk::Box* box = Gtk::manage(new Gtk::Box(Gtk::Orientation::ORIENTATION_VERTICAL));
    

    Gtk::Grid* grid = Gtk::manage(new Gtk::Grid);
    grid->set_border_width(10);
    grid->set_row_spacing(10);
    grid->set_column_spacing(10);
    grid->set_column_homogeneous(true);
    grid->set_row_homogeneous(true);
    grid->set_hexpand();
    grid->set_vexpand();


    scroll = Gtk::manage(new Gtk::ScrolledWindow);
    scroll->set_policy(Gtk::PolicyType::POLICY_NEVER, Gtk::PolicyType::POLICY_ALWAYS);
    scroll->set_resize_mode(Gtk::ResizeMode::RESIZE_IMMEDIATE);
    scroll->set_placement(Gtk::CornerType::CORNER_BOTTOM_LEFT);

    chat = Gtk::manage(new Gtk::Box(Gtk::Orientation::ORIENTATION_VERTICAL));

    Gtk::Separator* whitespace = Gtk::manage(new Gtk::Separator);

    Gtk::Label* welcome = Gtk::manage(new Gtk::Label);
    welcome->set_line_wrap();
    welcome->set_line_wrap_mode(Pango::WrapMode::WRAP_WORD_CHAR);
    welcome->set_markup("<b>Welcome to TalkzApp.</b>\nUse /connect to go online.");
    welcome->set_alignment(Gtk::Align::ALIGN_START, Gtk::Align::ALIGN_END);
    
    chat->pack_start(*whitespace);
    chat->pack_start(*welcome, false, true);

    scroll->add(*chat);


    Gtk::ScrolledWindow* input_control = Gtk::manage(new Gtk::ScrolledWindow);


    Gtk::TextView* input = Gtk::manage(new Gtk::TextView);
    input->set_wrap_mode(Gtk::WrapMode::WRAP_WORD_CHAR);
    input->set_border_width(5);

    input_buffer = Gtk::TextBuffer::create();

    input->set_buffer(input_buffer);


    Gtk::Button* send = Gtk::manage(new Gtk::Button("Send"));


    input_control->add(*input);

    grid->attach(*scroll, 0, 0, 5, 4);
    grid->attach(*input_control, 0, 4, 4, 1);
    grid->attach(*send, 4, 4, 1, 1);

    box->pack_start(*grid);
    box->show_all();

    window->add(*box);

    chat->signal_size_allocate().connect(sigc::ptr_fun(&auto_scroll));
	send->signal_clicked().connect(sigc::ptr_fun(&send_button_click));

	Glib::signal_timeout().connect(sigc::mem_fun(*client, &Client::receiver), 100);
}


void Screen::add_message(std::string& msg, std::string& nick) {
    Gtk::Label* label = Gtk::manage(new Gtk::Label);
    label->set_line_wrap();
    label->set_line_wrap_mode(Pango::WrapMode::WRAP_WORD_CHAR);
    label->set_markup("<b>" + nick + ":</b> " + msg);
    label->set_alignment(Gtk::Align::ALIGN_START, Gtk::Align::ALIGN_END);
    
    label->show();
    chat->pack_start(*label, false, true);
    
}

void Screen::log_message(std::string msg, LogType type) {
    Gtk::Label* label = Gtk::manage(new Gtk::Label);
    label->set_line_wrap();
    label->set_line_wrap_mode(Pango::WrapMode::WRAP_WORD_CHAR);

    switch(type) {
        case ERROR:
            label->set_markup("<b>ERROR!</b> " + msg);
            break;
        case WARNING:
            label->set_markup("<b>!</b> " + msg);
            break;
        case SUCCESS:
            label->set_markup("<b>" + msg + "</b>");
            break;
        default:
            label->set_markup(msg);
            break;
    }
    
    label->set_alignment(Gtk::Align::ALIGN_START, Gtk::Align::ALIGN_END);
    
    label->show();
    chat->pack_start(*label, false, true);
    
}