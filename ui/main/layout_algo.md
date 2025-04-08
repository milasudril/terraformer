# Layout run (Fixed widths)

1. Set cell sizes width zero width
	* Compute a suitable widget height
	* Set layout cell sizes to default

2. Set cell widths with available width
	* Apply cell widths specified by layout (return new layout width)
	* Recompute widget height given new width
	* Set layout cell heights to default (return new layout height)

3. Set cell heights with available heights
	* Apply cell heights specified by layout
	* Recompute widget width given new height
	* Set layout cell widths to default


