===============================================================================
  Video Stream Player (vsPlay)
===============================================================================

.. contents::

Overview
========

The vsPlay application is a tool designed to support the visualization and
exploitation of computer vision algorithms applied to motion imagery, with a
focus on narrow to medium FOV video at 480p or 720p resolution and frame rates
from 5 Hz to 30 Hz. (Other formats are supported |--| there are no imposed
restrictions on the video format |--| but functionality and/or performance may
be reduced.) The core algorithm support focuses on object detection and
tracking, and event detection.

The core functionality of vsPlay can be divided into a number of components:

* Video playback
* Video overlay visualization
* Archived data support
* External process support
* Integrated algorithms
* Supplemental tools
* Supplemental visualizations

Additional functionality may be provided through the use of plug-in modules,
which may substantially change the functionality of the application. The
nature and availability of such may vary widely depending on the customer and
target algorithms. This document attempts to focus on functionality that is
common to most distributions.

Interface Layout
================

The vsPlay interface is divided into a number of areas:

* Primary (video) view
* Menu bar
* Status bar
* Tool bars
* Secondary views

The menu bar, status bar, and video view (including the playback scrubber and
related controls), at the top, bottom and center of the main window
(respectively), are the only fixed components of the interface; the others may
be rearranged and/or detached as desired by the user, and will remember their
locations between executions. By default, all tool bars are visible at the top
of the main window, and a selection of secondary views are docked to the left
and right of the video view.

Video Playback
==============

The primary view of vsPlay is the video view. This presents the video imagery
along with any video overlays. Standard DVR-like controls allow the user to
play and pause the video, quickly "scrub" to a particular location, or change
the playback speed (both slow and fast speeds in both forward and reverse
directions are available). When paused, the user may also move forward and back
through the video either frame by frame, or in short jumps.

The view itself is also interactive, allowing visualization overlays to be
selected or the video imagery to be zoomed. Dragging with the right mouse
button zooms the video, while dragging with the middle mouse button pans.
Zooming may also be accomplished by dragging with the left mouse button to draw
a 'zoom target' box; on releasing the left mouse button, the view is zoomed to
minimally encompass the box. Zooming can also be accomplished using the mouse
wheel.

The area immediately below the video view contains additional playback controls
and indicators, showing the current playback speed / status, relative position
in the video (via the interactive scrubber), and current frame number. The
frame number control is also interactive, and can be used to jump to a specific
frame.

The current video time is displayed in the status bar. For video with embedded
time information, this shows the "wall clock time" corresponding to when the
currently visible video imagery was recorded. An estimate of the
`ground sample distance`_ is also displayed, if available.

If the video includes geospatial reference information, moving the mouse over
the video view additionally shows the estimated world location of the area
under the cursor.

.. notice::
   Video geospatial information is based on the meta data embedded in the
   video, and is often inaccurate. (The degree of inaccuracy varies depending
   on the video source, but is usually on the order of several meters.)

"Real Time" Playback
--------------------

Video playback speed is relative to the record speed, and will skip frames as
necessary to maintain the requested ratio between requested speed and actual
display of the video relative to the time at which the video was recorded. Note
that this means that playback can "stutter" or even pause if the recorded video
does so. As an exception, if vsPlay detects a large gap between recorded video
frames, it will skip playback ahead after a short delay in order to skip over
the gap without unreasonable delay.

"Live" Playback
---------------

When vsPlay is presenting video from a live streaming source, an additional
"live" playback option is available. In this mode, vsPlay displays video
relative to the time at which it is received by the application, rather than
the time at which it was recorded. This can be useful if the video stream is
subject to "bursts" and it is important to always see the most recent video (as
is often the case when monitoring a live stream).

An optional offset may be specified to show a position that is a specified
number of seconds before the latest available frame. This "buffer" is typically
used when running algorithms on a live stream to look at data that the
algorithms have had time to process.

Video Overlay Visualization
===========================

In additional to the raw video imagery, vsPlay uses overlay graphics to display
additional information, which may include algorithm visualizations and
interactive tools. When available with the video data, vsPlay uses homography
transformations to "stabilize" overlays against the scene when the camera is
moving.

The primary two visualizations supported are object detection and tracking
("tracks") and event detection. Each has three available display options that
may be controlled independently:

* "Trails" are displayed as a polyline that follows the historic location of
  the object using the estimated ground center point.

* Detection boxes ("heads") show an outline of the detection on the current
  frame.

* Labels give a brief (or not so brief, depending on options) description of
  the detection. These are styled like a cartoon "callout", with the tip
  indicating the location of the detection (or attached to one of the previous
  visualization types, when enabled).

Events support the first style of visualization only for events that are
associated with a track, in which case the corresponding track segment is
highlighted. Events are colored based on the highest confidence classification.

Additionally, vsPlay supports user-defined regions, which are simply free-form
regions that may be drawn on the video. These may be used as simple
annotations, or by other algorithms or visualization features.
See `Annotation Regions`_ for more details.

To help reduce clutter, vsPlay supports the user of user regions as either
filters or selectors. A Filter region hides other visualizations that are
contained within the filter region. A Selector region is the inverse; hiding
visualizations that are *outside* the Selector region. Multiple regions of each
type may be used, including combinations of both Filter and Selector regions.

Track Coloring Modes
--------------------

To aid in visualization and identification, vsPlay provides several modes for
coloring "track" entities, which can be selected and configured via
:action:`- Tracks` |->| :action:`track-color Change Color(s)`.

The most trivial is single color mode, in which all tracks use the same color.
The default mode is Object Classification, which assigns colors to each track
based on the classification with the highest confidence score (similar to event
coloring), using the default color if classifications are not available.

Dynamic Data mode colors individual sections of the track based on variable
data which has been provided with the track. The availability and exact nature
of such data depends on the provider, and might include for example computed
velocity or per-state detection confidence metrics.

Override options are provided which allow the coloring normally provided by the
preceding modes to be overridden depending on the source component of the
track's identifier. This can be used to visually differentiate tracks with
different sources.

Except for Dynamic Data mode, each mode provides a set of three colors. The Pen
color is used when drawing geometry (e.g. trails and heads) on the video, for
the swatches in the list views, and so forth. The Fore and Back colors are used
for labels associated with the detection, where such labels reflect the
detection classification coloring (e.g. in the main view).

Archived Data Support
=====================

There are two modes of data acquisition supported by vsPlay: archived and
streaming. Archived data acquisition involves loading stored or precomputed
data from archive files, which is useful for looking at data that was recorded
or computed at an earlier time. The three main data classes (video, tracks,
descriptors) each have a 'load from archive' action that is available when
suitable plugins have been loaded.

Video data provides imagery, and may include meta data about the imagery, such
as the time at which the imagery was captured, geospatial information about the
imagery, and homography (stabilization) information to spatially relate
consecutive video frames when camera motion is present. Track data provides
basic object detection information, which is presented in vsPlay as track
entities. Descriptors is a general categorization that covers "everything
else", and includes track object type classifications and event detections. In
some cases, additional data types may be defined by distribution specific
plugins.

The exact set of supported file formats may vary by distribution. A typical
vsPlay distribution includes support for at least the following data formats:

* **Video**

 - KWA archives

* **Tracks**

 - Generic CSV

 - Kitware KW18

* **Descriptors**

 - Kitware / VisGUI Saved Result Sets

 - Kitware Descriptor XML

 - Kitware P/V/O's

Some supported formats may provide more than one data type.

External Process Support
========================

Another option for data acquisition is to obtain it from a live system. When
these options are available, they are added to the respective menus. A data
source may provide more than one data type; many external data providers will
provide all three data types (video, tracks and descriptors).

The most common use of external process support is to allow a vsPlay instance
to connect to a live video feed and/or external detection algorithms. In some
cases, the ability to interact with such systems may also be available upon
connecting to the external process / system.

The nature of such systems can vary widely, and as such the availability of
external data providers is usually distribution specific.

Integrated Algorithms
=====================

In addition to external processes, vsPlay also supports the execution of
algorithms within its own process space. As the vsPlay application itself does
not provide any significant computer vision algorithms, most such algorithms
would be provided as add-ons, and hence are distribution specific.

As an exception, the standard vsPlay distribution provides a very rudimentary
event detection system in the form of "tripwires". These are a type of
annotation region (similar to Selector / Filter regions, created and
manipulated using the same tools) that interact with tracks to detect simple
boundary crossing. Open tripwires generate a "tripwire" event whenever a track
crosses the region. Closed tripwires generate an "entering" or "leaving" event
when a track enters or leaves the region, respectively.

Supplemental Tools
==================

Classification Filters
----------------------

Tracks and Events in vsPlay both implement a concept of "classification". A
classification is information about the detection, usually provided by the
detecting algorithm (or, in some cases, by the operator). Track classification
is optional, and identifies the type of entity being tracked (currently,
"Person", "Vehicle", or "Other"). Event classification is mandatory (events
without a classification are not displayed in the main view), and is broken
into groups. Some classifications (e.g. those related to the in-process
tripwire algorithm) are built in, while others are dynamically defined. Event
classifications might include 'vehicle making U-turn', 'person walking', or
'person juggling flaming batons'.

The term "classification" is used instead of "type" to indicate that the
classification is an estimate, which as a consequence has an associated
confidence score and may not be unique (i.e. an entity may have several
classifications with varying confidence scores).

Detection classifications and confidences are used as a primary means of
filtering, via the Filters panel (:action:`- Events` |->| :action:`filter-show
Show Filter Window`). The panel provides a collapsible list of controls for
known classifications and allows the user to individually select general
visibility and threshold levels for each. (Event groups also provide a
convenience visibility toggle for all events in the the group.) Filters are
low-pass by default, causing detections for which the corresponding
classification is below the specified threshold to be hidden. This can be
inverted using the check box adjacent to the threshold spin box.

.. notice::
   Event classification filters are hidden initially, and are only made visible
   when their use is relevant (i.e. detections containing event classifications
   are available or expected to become available).

The default threshold for track classifications is 0.0. The default threshold
for event classifications is 0.1. The :action:`load Load` and :action:`save
Save` tool buttons at the bottom of the panel allow filter settings to be
loaded from or saved to a file. The threshold for all event classifications may
also be changed globally via :action:`- Events` |->| :action:`- Set Threshold`.

The thresholds and visibility states work together to determine the most
appropriate classification for a detection, if any, which may then be displayed
to the user in a label or the respective detection list, and to choose the
color in which to display the detection in other views. A detection that has
no classification that passes the filters may be hidden (or shown only if a
respective "show hidden" option is selected). Otherwise, the classification
with the highest confidence *that is not excluded by the filters* is used. Be
aware that this classification might *not* be the classification with the
highest score when filters are not considered.

Ruler Tool
----------

The ruler tool is used to measure distances on a video. When active, a line is
drawn over the video, providing two handles at either end which may be dragged
to change the measurement points. If the video provides a
`ground sample distance`_ estimate, the estimated physical distance between the
selected points (in meters) is also displayed.

Annotation Regions
------------------

The vsPlay application allows the user to create arbitrary contours, or
regions, which are stabilized with respect to the video imagery (when the video
includes stabilization information). Regions may be managed using the
`Region List`_, and come in four styles:

* An Annotation region is used to draw markings on video, in order to visually
  call attention to some item or otherwise "make notes". While these typically
  serve no other purpose, distribution specific add-on components may make
  additional use of them.

* A Tripwire region is used by the built-in tripwire detection algorithm, which
  will generate events whenever a track crosses the boundary. Closed boundaries
  generate 'enter' and 'exit' events, while open boundaries generate 'tripwire'
  events.

* A Filter region is used to exclude detections from an area of video.
  Detections (and portions of detections) within a filter are hidden.

* A Selector region is a conceptual opposite of a filter; rather than hiding
  detections within the region, detections that are *not* contained in a
  selector are hidden.

Multiple Filter and Selector regions may be active at any time. A detection is
visible if it is within one or more Selector regions (or if no Selector regions
are active), is not within a Filter region, and is not hidden by non-spatial
filters or display options. By default, filtered regions of the video are
dimmed to indicate the action of spatial filtering.

Regions are created using the :action:`path-draw Draw` action. When drawing,
click to add individual points, or hold the left mouse button to draw freehand
shapes. When done, click the right mouse button to enter edit mode, which
allows points to be moved or deleted. Right click a second time to complete the
region. The :action:`path-close Close Path` action may be used at any time to
end drawing and create a closed region by connecting the start and end points.

.. notice::
   Some region types (Filters, Selectors) must be closed, and will be closed
   automatically when editing is concluded. If the region does not contain
   enough points, the region will not be created.

Regions may be drawn while the video is playing. In this case, the region in
progress will be stabilized in the same manner as completed regions.

Manual Event Creation
---------------------

Supplemental Visualizations
===========================

Track List
----------

Event List
----------

Event Information
-----------------

Region List
-----------

Appendix I: Menu Actions
========================

Video Menu
----------

:icon:`playback-play` Play
  Plays the video in the normal (forward) direction at the same speed at which
  the video was recorded. If already playing at this rate, slow playback by a
  factor of two until the minimum speed is reached, then 'wrap back' to normal
  speed.

:icon:`playback-pause` Pause
  Pauses the video playback.

:icon:`playback-play-reverse` Play Reversed
  Plays the video in reverse at the same speed at which the video was recorded.
  Like Play, selecting this action when already playing in reverse will cycle
  through the available "slow" speeds.

:icon:`playback-fast-backward` Fast Backward
  Plays the video in reverse at an accelerated rate. The rate is relative to
  the speed at which the video was recorded. This may be selected (clicked)
  multiple times to increase the speed up to the maximum, at which point the
  action 'wraps back' to twice normal speed.

:icon:`playback-fast-forward` Fast Forward
  Plays the video in the normal (forward) direction at an accelerated rate. The
  behavior is otherwise the same as for Fast Backward.

:icon:`playback-stop` Stop
  Stops the video playback and reset to the beginning.

:icon:`blank` Resume
  Resumes playback of the video (e.g. after pausing) at the previous speed.

:icon:`playback-stop` Decrease Speed
  Decreases the speed of video playback by a factor of 2.

:icon:`playback-stop` Increase Speed
  Increases the speed of video playback by a factor of 2.

:icon:`playback-frame-backward` Frame Backward
  Steps the video backward by one frame.

:icon:`playback-frame-forward` Frame Forward
  Steps the video forward by one frame.

:icon:`playback-skip-backward` Skip Backward
  Skips backward in the video a few seconds.

:icon:`playback-skip-forward` Skip Forward
  Skips forward in the video a few seconds.

:icon:`playback-play-live` Live
  Selects `"Live" Playback`_ mode.

:icon:`view-reset` Reset View
  Resets the zoom and pan of the video view so that the entire video frame is
  visible and centered, with minimal padding.

:icon:`blank` Set Live Offset...
  Sets the offset that is applied to `"Live" Playback`_ mode.

:icon:`blank` Resampling Mode
  Selects the image scaling algorithm that is applied to the video imagery. The
  available options are Nearest, Linear, and Bicubic. Nearest disables
  interpolation and produces "blocky" pixels, while Bicubic (default) typically
  produces the best result with the fewest artifacts.

:icon:`load-video` Load Archive
  Load video from a file on disk. The available formats may depend on what
  plugins are available.

:icon:`quit` Quit
  Exits the application.

Tracks Menu
-----------

:icon:`track-show` Show Tracks
  Toggles display of object tracking detection trails.

:icon:`track-show-boxes` Show Entity Boxes
  Toggles display of object tracking detection heads.

:icon:`track-show-id` Show ID's
  Toggles display of labels showing the track identifier for tracking
  detections.

:icon:`track-show-pvo` Show PVO's
  Toggles display of labels showing the track object-type classification for
  tracking detections.

:icon:`track-color` Change Color(s)
  Allows changing the track coloring mode and colors.

:icon:`blank` Set Trail Length
  Sets the size of the historic window for which trails are displayed.

:icon:`track-list` Show Track List
  Toggles visibility of the `Track List`_ panel.

:icon:`load-tracks` Load Archive
  Load tracks from a file on disk. The available formats may depend on what
  plugins are available.

Events Menu
-----------

:icon:`blank` Show All Person Events
  Sets the `Classification Filters`_ visibility state for all event types in
  the Person group to **on**.

:icon:`blank` Hide All Person Events
  Sets the `Classification Filters`_ visibility state for all event types in
  the Person group to **off**.

:icon:`blank` Show All Vehicle Events
  Sets the `Classification Filters`_ visibility state for all event types in
  the Vehicle group to **on**.

:icon:`blank` Hide All Vehicle Events
  Sets the `Classification Filters`_ visibility state for all event types in
  the Vehicle group to **off**.

:icon:`load` Load Filter Settings
  Loads `Classification Filters`_ settings from a file on disk.

:icon:`save` Save Filter Settings
  Saves `Classification Filters`_ settings to a file on disk.

:icon:`blank` Set Threshold
  Sets the threshold for all event `Classification Filters`_ to a specified
  value.

:icon:`filter-show` Show Filter Window
  Toggles visibility of the `Classification Filters`_ panel.

:icon:`event-show-tracks` Show Event Tracks
  Toggles display of object event detection trails for events associated with
  one or more tracks.

:icon:`event-show-boxes` Show Event Boxes
  Toggles display of object event detection heads.

:icon:`event-show-label` Show Labels
  Toggles display of labels showing the event type classification for event
  detections. The classification(s) with the highest confidence are shown, in
  descending order of confidence.

:icon:`event-show-probability` Show Probability
  Toggles display of the confidence score in the event classification labels.

:icon:`blank` Show Event List
  Toggles visibility of the `Event List`_ panel for unadjudicated events.

:icon:`blank` Show Verified Event List
  Toggles visibility of the `Event List`_ panel for Verified events.

:icon:`blank` Show Rejected Event List
  Toggles visibility of the `Event List`_ panel for Rejected events.

:icon:`blank` Show Event Info
  Toggles visibility of the `Event Information`_ panel.

Descriptors Menu
----------------

:icon:`load-events` Load Archive
  Load descriptors from a file on disk. The available formats may depend on
  what plugins are available.

Regions Menu
------------

:icon:`path-draw` Draw
  Starts or cancel region drawing.

:icon:`path-close` Close Path
  Completes region drawing by closing the start and end points of the path to
  form a closed region.

:icon:`path-clear` Clear Selected
  Removes the user regions which are currently selected in the `Region List`_.

:icon:`path-clear-all` Clear All
  Removes all user regions.

:icon:`mask-show-filtering` Show Filter Mask
  Toggles visibility of the spatial filtering mask.

.. TODO: Not implemented yet
.. :icon:`mask-color` Change Mask Color
..   Changes the color of the spatial filtering mask.

:icon:`path-list` Show Region List
  Toggles visibility of the `Region List`_ panel.

Tools Menu
----------

:icon:`measure` Show Ruler
  Toggles display of the `Ruler Tool`_.

:icon:`blank` Create Event Menu
  :icon:`event-draw` Draw Event
    Creates a new manual event by drawing a free-form shape.

  :icon:`event-create-box` Create Boxed Event
    Creates a new manual event by drawing a rectangle.

  :icon:`event-create-quick` Create Event (Quick)
    Creates a new manual event using the previous shape.

  :icon:`blank` Create Full Frame Event
    Creates a new manual event with no location.

  :icon:`blank` <*Event Types*>
    Selects the type of manual event to be created by the above tools.

.. TODO: query formulation, KML export, report generation

Help Menu
---------

.. TODO: use 'help-manual' icon

:icon:`blank` User Manual
  Opens the vsPlay user manual (i.e. this document).

:icon:`vsPlay` About Video Stream Player
  Shows copyright and version information about the application.

Appendix II: Tool Bar Actions
=============================

Most of the tool bar actions duplicate menu actions. The function of these is
identical to the corresponding menu action.

:icon:`blank` Region Type
  Select the type of region to be drawn (combo box next to the
  :action:`path-draw Draw` action).

.. |->| unicode:: U+02192 .. right arrow
.. |--| unicode:: U+02014 .. em dash

.. _ground sample distance:
   http://en.wikipedia.org/wiki/Ground_sample_distance
