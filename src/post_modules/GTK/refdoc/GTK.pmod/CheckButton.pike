//! Check buttons inherent many properties and functions from the the
//! toggle buttons, but look a little different. Rather than
//! being buttons with text inside them, they are small squares with
//! the text to the right of them. These are often used for toggling
//! options on and off in applications.
//!@code{ GTK.Check_button( "title" )@}
//!@xml{<image src='../images/gtk_checkbutton.png'/>@}
//!
//!
//!
inherit ToggleButton;

static CheckButton create( string|void label )
//! The argument, if specified, is the label of the item.
//! If no label is specified, use object->add() to add some
//! other widget (such as an pixmap or image widget)
//!
//!
