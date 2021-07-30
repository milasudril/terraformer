#ifndef GUI_HPP
#define GUI_HPP

class Gui
{
public:
	using RenderCallback = UniqueFunction<void()(GuiPixel const* srcbuff, int width, int height)>;

	void update_canvas_size(int width, int height);
	void update() const;
	void send_keyboard_event(Scancode scancode, KeyState state, CharCodepoint mapped_codepoint);
	void send_mouse_event(double x, double y, int button, MouseButtonState state);
	void send_midi_event(unsigned int device_id, MidiEvent event);
	void send_frame_start_event(uint64_t framecounter);
	void set_render_callback(UniueFunction<>)

private:
	std::vector<std::unique_ptr<Widget>> m_widgets;

};

#endif