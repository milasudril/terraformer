#ifndef EVENT_DISPATCHER_HPP
#define EVENT_DISPATCHER_HPP

class EventDispatcher
{
public:
	using DisplayCallback = UniqueFunction<void()(GuiPixel const* srcbuff, int width, int height)>;

	void update_canvas_size(int width, int height);
	void update() const;
	void send_typing_event(DeviceId device, Scancode scancode, ButtonState state, CharCodepoint mapped_codepoint);
	void send_mouse_event(double x, double y, int button, ButtonState state);
	void send_scroll_event(double dx, double dy);
	void send_midi_event(DeviceId device, MidiEvent event);
	void send_frame_start_event(uint64_t framecounter);

	void set_display_callback(DisplayCallback&& cb);

	void bind(Widget& widget, DeviceId device);
	void unbind(Widget& widget, DeviceId device);
	void unbind(Widget& widget);

private:
	DisplayCallback m_disp_callback;
	std::vector<std::reference_wrapper<Widget>> m_widgets;
	std::map<DeviceId, std::vector<std::reference_wrapper<Widget>>> m_sensitive_widgets;
};

#endif