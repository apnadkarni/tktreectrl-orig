namespace eval DemoHeaders {}

proc DemoHeaders {} {
    DemoHeaders::Run
    return
}

proc DemoHeaders::Run {} {
    set T [DemoList]

    $T configure \
	-showroot no -xscrollsmoothing yes -yscrollsmoothing yes \
	-selectmode multiple -xscrollincrement 20 -canvaspadx 40

    #
    # Create one locked column on each side plus 8 non-locked columns
    #

    set itembg {linen {} #e0e8f0 {}}

    $T column create -text "Left" -tags Cleft -width 80 -justify center \
	-gridrightcolor gray90 -itembackground $itembg \
	-lock left -arrow none -arrowside left \
	-visible no

    for {set i 1} {$i <= 8} {incr i} {
	$T column create -text "C$i" -tags C$i -width 80 -justify center \
	    -gridrightcolor gray90 -itembackground $itembg
    }

    $T column create -text "Right" -tags Cright -width 80 -justify center \
	-gridrightcolor gray90 -itembackground $itembg  \
	-lock right -visible no

    #
    # Create an image element to use as the sort arrow for some header
    # styles.
    #

    InitSortImages blue
    $T element create header.sort image -statedomain header \
	-image {::DemoHeaders::arrow-down down ::DemoHeaders::arrow-up up}

    #
    # Create a style for our custom headers,
    # a raised border with centered text.
    #

    $T element create header.border border -statedomain header \
	-background $::SystemButtonFace \
	-relief {sunken pressed raised {}} -thickness 2 -filled yes
    $T element create header.text text -statedomain header \
	-lines 1 -fill black

    set S [$T style create header1 -orient horizontal -statedomain header]
    $T style elements $S {header.border header.text header.sort}
    $T style layout $S header.border -detach yes -indent no -iexpand xy
    $T style layout $S header.text -center xy -padx 6 -pady 2 -squeeze x
    $T style layout $S header.sort -expand nws -padx {0 6} \
	-visible {no {!down !up}}

    #
    # Create a style for our custom headers,
    # a light-blue rounded rectangle with centered text.
    #

    $T element create header.rrect rect -statedomain header \
	-rx 9 -fill {
	    #cee8f0 active
	    #87c6da pressed
	    #87c6da up
	    #87c6da down
	    {light blue} {}
	}

    set S [$T style create header2 -orient horizontal -statedomain header]
    $T style elements $S {header.rrect header.text header.sort}
    $T style layout $S header.rrect -detach yes -iexpand xy -padx {1 0} -pady 1
    $T style layout $S header.text -center xy -padx 6 -pady 4 -squeeze x
    $T style layout $S header.sort -expand nws -padx {0 6} \
	-visible {no {!down !up}}

if 0 {
    #
    # Create a style for our custom headers,
    # a header element with centered text.
    #

    $T element create header.header header -thickness 1

    set S [$T style create header3 -orient horizontal -statedomain header]
    $T style elements $S {header.header header.text}
    $T style layout $S header.header -union header.text -iexpand news
    $T style layout $S header.text -expand wens -padx 6 -pady 2 -squeeze x ; # $T style layout $S header.text -expand ns -center x -padx 6 -pady 2 -squeeze x
}

    #
    # Create a style for our custom headers,
    # a header element with a checkbox image and centered text.
    #

    InitPics *checked

    $T header state define CHECK
    $T element create header.header header -statedomain header
    $T element create header.check image -statedomain header \
	-image {checked CHECK unchecked {}}
    set S [$T style create header4 -statedomain header]
    $T style elements $S {header.header header.check header.text}
    $T style layout $S header.header -union {header.check header.text} -iexpand news
    $T style layout $S header.check -expand nes -padx {6 0}
    $T style layout $S header.text -center xy -padx 6 -squeeze x

    #
    # Create a style for our custom headers,
    # a gradient-filled rectangle with centered text.
    #

    $T gradient create Gnormal -orient vertical -stops {{0.0 white} {0.5 gray87} {1.0 white}} -steps 6
    $T gradient create Gactive -orient vertical -stops {{0.0 white} {0.5 gray90} {1.0 white}} -steps 6
    $T gradient create Gpressed -orient vertical -stops {{0.0 white} {0.5 gray82} {1.0 white}} -steps 6
    $T gradient create Gsorted -orient vertical -stops {{0.0 white} {0.5 {sky blue}} {1.0 white}} -steps 6
    $T gradient create Gactive_sorted -orient vertical -stops {{0.0 white} {0.5 {light blue}} {1.0 white}} -steps 6
    $T gradient create Gpressed_sorted -orient vertical -stops {{0.0 white} {0.5 {sky blue}} {1.0 white}} -steps 6
    $T element create header.rect1 rect  -statedomain header \
    -fill {
	Gactive_sorted {active up}
	Gpressed_sorted {pressed up}
	Gactive_sorted {active down}
	Gpressed_sorted {pressed down}
	Gsorted up
	Gsorted down
	Gactive active
	Gpressed pressed
	Gnormal {}
    } -outline {
	{sky blue} up
	{sky blue} down
	gray {}
    } -outlinewidth 1 -open {
	nw !pressed
    }

    set S [$T style create header5 -orient horizontal -statedomain header]
    $T style elements $S {header.rect1 header.text header.sort}
    $T style layout $S header.rect1 -detach yes -iexpand xy
    $T style layout $S header.text -center xy -padx 6 -pady 2 -squeeze x
    $T style layout $S header.sort -expand nws -padx {0 6} \
	-visible {no {!down !up}}

    #
    # Create a style for our custom headers,
    # a gradient-filled rectangle with centered text.
    #

    $T gradient create G_orange1 -orient vertical -steps 4 \
	-stops {{0 #fde8d1} {0.3 #fde8d1} {0.3 #ffce69} {0.6 #ffce69} {1 #fff3c3}}
    $T gradient create G_orange2 -orient vertical -steps 4 \
	-stops {{0 #fffef6} {0.3 #fffef6} {0.3 #ffef9a} {0.6 #ffef9a} {1 #fffce8}}

    $T element create orange.outline rect -statedomain header \
	-outline #ffb700 -outlinewidth 1 \
	-rx 1 -open {
	    nw !pressed
	}
    $T element create orange.box rect -statedomain header \
	-fill {
	    G_orange1 active
	    G_orange1 up
	    G_orange1 down
	    G_orange2 {}
	}

    set S [$T style create header6 -orient horizontal -statedomain header]
    $T style elements $S {orange.outline orange.box header.text header.sort}
    $T style layout $S orange.outline -union orange.box -ipadx 2 -ipady 2
    $T style layout $S orange.box -detach yes -iexpand xy
    $T style layout $S header.text -center xy -padx 6 -pady 4 -squeeze x
    $T style layout $S header.sort -expand nws -padx {0 6} \
	-visible {no {!down !up}}

    #
    # Configure 3 rows of column headers
    #

    set S header2

if 1 {
    $T header configure first -tags header1
} else {
    $T header configure first -ownerdrawn yes -tags header1
}
    set H header1
    $T header configure $H all -arrowgravity right -justify center
    $T header style set $H all $S
    $T header span $H all 4
    foreach {C text} [list Cleft Left C1 A C5 H Cright Right] {
	$T header configure $H $C -text $text
	$T header text $H $C $text
    }

if 1 {
    set H [$T header create -tags header2]
} else {
    set H [$T header create -ownerdrawn yes -tags header2]
}
    $T header configure $H all -arrowgravity right -justify center
    $T header style set $H all $S
    $T header span $H all 2
    foreach {C text} [list Cleft Left C1 B C3 C C5 I C7 J Cright Right] {
	$T header configure $H $C -text $text
	$T header text $H $C $text
    }

if 1 {
    set H [$T header create -tags header3]
} else {
    set H [$T header create -ownerdrawn yes -tags header3]
}
    $T header configure $H all -arrowgravity right -justify center
    $T header style set $H all $S
    foreach {C text} [list Cleft Left C1 D C2 E C3 F C4 G C5 K C6 L C7 M C8 N Cright Right] {
	$T header configure $H $C -text $text
	$T header text $H $C $text
    }

    #
    # Create a 4th row of column headers to test embedded windows.
    #

    $T element create header.window window -statedomain header -clip yes
    $T element create header.divider rect -statedomain header -fill gray -height 2

    set S [$T style create headerWin -orient horizontal -statedomain header]
    $T style elements $S {header.divider header.window}
    $T style layout $S header.divider -detach yes -expand n -iexpand x
    $T style layout $S header.window -iexpand x -squeeze x -padx 1 -pady {0 2}

if 1 {
    set H [$T header create -tags header4]
} else {
    set H [$T header create -ownerdrawn yes -tags header4]
}
    $T header dragconfigure $H -enable no
    $T header style set $H all $S
    foreach C [$T column list] {
        set f [frame $T.frame${H}_$C -borderwidth 0]
	set w [entry $f.entry -highlightthickness 1]
	$w insert end $C
	$T header element configure $H $C header.window -window $f
    }

    #
    #
    #

    scan [$T column bbox {first lock none}] "%d %d %d %d" left top right bottom
    scan [$T column bbox {last lock none}] "%d %d %d %d" left2 top2 right2 bottom2
    set width [expr {$right2 - $left}]

    $T item state define current

    $T element create theme.rect rect \
	-fill {{light blue} current white {}} \
	-outline gray50 -outlinewidth 2 -open s
    $T element create theme.text text \
	-lines 0 -width $width
    $T element create theme.button window -clip yes
    set S [$T style create theme -orient vertical]
    $T style elements $S {theme.rect theme.text theme.button}
    $T style layout $S theme.rect -detach yes -iexpand xy
    $T style layout $S theme.text -padx 4 -pady 3
    $T style layout $S theme.button -expand we -pady {3 6}

    NewButtonItem "" \
	"Use no style, just the built-in header background, sort arrow and text." \
	no
    NewButtonItem header1 \
	"Use the 'header1' style, consisting of a border element for the background and an image for the sort arrow." \
	yes black
    NewButtonItem header2 \
	"Use the 'header2' style, consisting of a rounded rectangle element for the background and an image for the sort arrow." \
	yes blue
    NewButtonItem header4 \
	"Use the 'header4' style, consisting of a header element to display the background and sort arrow, and an image element serving as a checkbutton." \
	no
    NewButtonItem header5 \
	"Use the 'header5' style, consisting of a gradient-filled rectangle element for the background and an image for the sort arrow." \
	yes #0080FF
    NewButtonItem header6 \
	"Use the 'header6' style, consisting of a gradient-filled rectangle element for the background and an image for the sort arrow." \
	yes orange

    $T item state set styleheader2 current

    #
    # Create 100 regular non-locked items
    #

    $T element create item.sel rect \
	-fill {gray {selected !focus} blue selected}
    $T element create item.text text \
	-text "Item" -fill {white selected}

    set S [$T style create item]
    $T style elements $S {item.sel item.text}
    $T style layout $S item.sel -detach yes -iexpand xy
    $T style layout $S item.text -expand news -padx 2 -pady 2

    $T column configure !tail -itemstyle $S
    $T item create -count 100 -parent root

    #
    # Set binding scripts to handle the <Header> dynamic event
    #

    variable Sort
    set Sort(header) ""
    set Sort(column) ""
    foreach C [$T column list] {
	set Sort(direction,$C) down
    }

    # The <Header-state> event is generated in response to Motion and
    # Button events in headers.
    $T notify install <Header-state>
    $T notify bind $T <Header-state> {
	DemoHeaders::HeaderState %H %C %s
    }

    # The <Header-invoke> event is generated when the left mouse button is
    # pressed and released over a column header.
    $T notify bind $T <Header-invoke> {
	DemoHeaders::HeaderInvoke %H %C
    }

    $T notify bind $T <ColumnDrag-begin> {
	DemoHeaders::ColumnDragBegin %H %C
    }

    $T notify configure DontDelete <ColumnDrag-receive> -active no
    $T notify bind $T <ColumnDrag-receive> {
	DemoHeaders::ColumnDragReceive %H %C %b
    }

    bindtags $T [list $T DemoHeaders TreeCtrl [winfo toplevel $T] all]
    bind DemoHeaders <ButtonPress-1> {
	DemoHeaders::ButtonPress1 %x %y
    }

    return
}

proc DemoHeaders::NewButtonItem {S text args} {
    set T [DemoList]
    set I [$T item create -parent root -tags [list style$S config]]
    $T item style set $I C1 theme
    $T item span $I all [$T column count {lock none}]
    $T item text $I C1 $text
    frame $T.frame$I -borderwidth 0
    $::buttonCmd $T.frame$I.button -text "Configure headers" \
	-command [eval list [list DemoHeaders::ChangeHeaderStyle $S] $args]
    $T item element configure $I C1 theme.button -window $T.frame$I
    return
}

proc DemoHeaders::InitSortImages {color} {
    set img ::DemoHeaders::arrow-down
    image create photo $img
    $img put [list [string repeat "$color " 9]] -to 0 0
    $img put [list [string repeat "$color " 7]] -to 1 1
    $img put [list [string repeat "$color " 5]] -to 2 2
    $img put [list [string repeat "$color " 3]] -to 3 3
    $img put [list [string repeat "$color " 1]] -to 4 4

    set img ::DemoHeaders::arrow-up
    image create photo $img
    $img put [list [string repeat "$color " 1]] -to 4 0
    $img put [list [string repeat "$color " 3]] -to 3 1
    $img put [list [string repeat "$color " 5]] -to 2 2
    $img put [list [string repeat "$color " 7]] -to 1 3
    $img put [list [string repeat "$color " 9]] -to 0 4
    return
}

proc DemoHeaders::ChangeHeaderStyle {style ownerDrawn {sortColor ""}} {
    variable HeaderStyle
    variable Sort
    set T [DemoList]
    if {$sortColor ne ""} {
	InitSortImages $sortColor
    }
    set HeaderStyle $style
    set S $HeaderStyle
    foreach H [$T header id !header4] {
	$T header style set $H all $S
	if {$S ne ""} {
	    $T header configure all all -textpadx 6
	    foreach C [$T column list] {
		$T header text $H $C [$T header cget $H $C -text]
	    }
	}
if 0 {
	$T header configure $H -ownerdrawn $ownerDrawn
}
    }
    if {$Sort(header) ne ""} {
	ShowSortArrow $Sort(header) $Sort(column)
    }
    $T item state set {state current} !current
    $T item state set style$style current
    return
}

# This procedure is called to handle the <Header-state> event generated by
# the treectrl.tcl library script.
proc DemoHeaders::HeaderState {H C state} {
    return
}

# This procedure is called to handle the <Header-invoke> event generated by
# the treectrl.tcl library script.
# If the given column header is already displaying a sort arrow, the sort
# arrow direction is toggled.  Otherwise the sort arrow is removed from all
# other column headers and displayed in the given column header.
proc DemoHeaders::HeaderInvoke {H C} {
    variable Sort
    set T [DemoList]
#    if {![$T item tag expr $I header3]} return
    if {$Sort(header) eq ""} {
	ShowSortArrow $H $C
    } else {
	if {[$T header compare $H == $Sort(header)] &&
		[$T column compare $C == $Sort(column)]} {
	    ToggleSortArrow $H $C
	} else {
	    HideSortArrow $Sort(header) $Sort(column)
	    ShowSortArrow $H $C
	}
    }
    set Sort(header) $H
    set Sort(column) $C
    return
}

# Sets the -arrow option of a column header to 'up' or 'down'.
proc DemoHeaders::ShowSortArrow {H C} {
    variable Sort
    set T [DemoList]
    $T header configure $H $C -arrow $Sort(direction,$C)
    return
}

# Sets the -arrow option of a column header to 'none'.
proc DemoHeaders::HideSortArrow {H C} {
    set T [DemoList]
    $T header configure $H $C -arrow none
    return
}

proc DemoHeaders::ToggleSortArrow {H C} {
    variable Sort
    if {$Sort(direction,$C) eq "up"} {
	set Sort(direction,$C) down
    } else {
	set Sort(direction,$C) up
    }
    ShowSortArrow $H $C
    return
}

# This procedure is called to handle the <ColumnDrag-begin> event generated
# by the treectrl.tcl library script.
proc DemoHeaders::ColumnDragBegin {H C} {
    set T [DemoList]
    $T header dragconfigure all -draw yes
    if {[$T header compare $H > header1]} {
	$T header dragconfigure header1 -draw no
    }
    if {[$T header compare $H > header2]} {
	$T header dragconfigure header2 -draw no
    }
    return
}

# This procedure is called to handle the <ColumnDrag-receive> event generated
# by the treectrl.tcl library script.
proc DemoHeaders::ColumnDragReceive {H C b} {
    set T [DemoList]

    # Get the range of columns in the span of the dragged header.
    set span [$T header span $H $C]
    set last [$T column id "$C span $span"]
    set columns [$T column id "range $C $last"]

    set span1 [$T header span header1]
    set span2 [$T header span header2]
    set text1 [$T header text header1]
    set text2 [$T header text header2]

    set columnLeft [$T column id "first lock none"]

    foreach C $columns {
	$T column move $C $b
    }

    if {[$T header compare $H > header1]} {
	foreach span $span1 text $text1 C [$T column list] {
	    $T header span header1 $C $span
	    $T header text header1 $C $text
	    $T header configure header1 $C -text $text
	}
    }
    if {[$T header compare $H > header2]} {
	foreach span $span2 text $text2 C [$T column list] {
	    $T header span header2 $C $span
	    $T header text header2 $C $text
	    $T header configure header2 $C -text $text
	}
    }

    # For each of the items displaying a button widget to change the header
    # style, transfer the style from the old left-most column to the new
    # left-most column.
    if {[$T column compare $columnLeft != "first lock none"]} {
	foreach I [$T item id "tag config"] {
	    TransferItemStyle $T $I $columnLeft "first lock none"
	}
    }

    return
}

proc DemoHeaders::TransferItemStyle {T I Cfrom Cto} {
    set S [$T item style set $I $Cfrom]
    $T item style set $I $Cto $S
    foreach E [$T item style elements $I $Cfrom] {
	foreach info [$T item element configure $I $Cfrom $E] {
	    lassign $info option x y z value
	    $T item element configure $I $Cto $E $option $value
	}
    }
    $T item style set $I $Cfrom ""
    return
}

proc DemoHeaders::ButtonPress1 {x y} {
    set T [DemoList]
    $T identify -array id $x $y
    if {$id(where) eq "header" && $id(element) eq "header.check"} {
	$T header state forcolumn $id(header) $id(column) ~CHECK
	return -code break
    }
    return
}
