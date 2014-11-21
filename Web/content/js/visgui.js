/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

var IqrClassification = {
  PositiveExample: 0,
  NegativeExample: 1,
  UnclassifiedExample: 2
};

/******************************************************************************
 * Helper method: does nothing.
 * @private
 *****************************************************************************/
var noop = function () {};

/******************************************************************************
 * vgWeb prototype object.
 * @class vgWeb
 * @constructor
 *****************************************************************************/
var vgWeb = function () {
  var pv = {},
      timeoutInstance = null,
      shouldRunAnimation = false,
      serviceURL = location.protocol + "//" +
                   location.hostname + ":" + location.port + "/paraview",
      refreshIntervalIds = [],
      config = {
        sessionManagerURL: serviceURL,
        name: "File loader viz",
        description: "3D visualization with ParaViewWeb 2.0",
        application: "loader"
      },
      viewportId = ".videoPlayer",
      viewport = $(viewportId),
      videoPlayerId = null,
      map,
      markers = null,
      markerCollection = {},
      // Transform from WGS 1984
      fromProjection = new OpenLayers.Projection("EPSG:4326"),
      // to target projection (which depends on map type)
      toProjection = null,
      tracksMetadataMap = {},
      that = null,
      colorStops = [
        {value: 0.97,
         color: 'green'},
        {value: 0.34,
         color: 'yellow'},
        {value: 0,
         color: 'red'}
      ],
      currentStartTime = -1,
      currentEndTime = -1,
      currentRow = null;

  /****************************************************************************
   * Set state of 'busy' indicator.
   * @private
   * @param {boolean} state True to show busy indicator, false to hide it.
   ***************************************************************************/
  function setBusy (state) {
    if (state) {
      // $(".loading").show(); TODO?
    } else {
      // $(".loading").hide(); TODO?
    }
  }

  /****************************************************************************
   * Sort the result ID list in-place by the given field(s).
   * @param {array} ids The result ID list to be sorted
   * @param {array} results The result list
   * @param {array} [field=["rank"]] The field(s) to sort on
   * @param {boolean} [descending=false] Sort descending (rather than ascending)
   * @private
   ***************************************************************************/
  function sortResults (ids, results, fields, descending) {
    fields = fields || ['rank'];
    descending = descending || [false];


    ids.sort(function (a, b) {
      var i, key, val;
      for (i in fields) {
        key = fields[i];
        val = descending[i] ? -1 : 1;
        if (results[a][key] != results[b][key]) {
          return results[a][key] > results[b][key] ? val : -val;
        }
      }
      return 0;
    });
  }

  /****************************************************************************
   * Construct a generic error callback.
   * @private
   * @param {string} msg Message to show in console on each error callback.
   ***************************************************************************/
  function createErrorCallback (msg) {
    return function(e) {
      console.log(msg);
      if (e !== null) {
        for (line in e.detail) {
          console.log(e.detail[line]);
        }
      }
    }
  }

  /****************************************************************************
   * ParaView session error callback.
   * @private
   * @param code This parameter is ignored.
   * @param reason This parameter is written to the console.
   ***************************************************************************/
  function pvErrorCallback(code, reason) {
    setBusy(false);
    console.log(reason);
  }

  /****************************************************************************
   * ParaView session connect callback, called when the ParaView session is
   * connected and ready.
   * @private
   * @param connection The connection configuration object.
   ***************************************************************************/
  function pvConnectCallback (connection) {
    setBusy(false);

    // Save session for later
    pv.session = connection.session;

    // Create viewport
    pv.viewport = paraview.createViewport({ session: connection.session });
    pv.viewport.bind(viewportId);

    // Initialize visualization
    pv.session.call("pv:createVideoPlayer").then(
      vpCreateCallback,
      createErrorCallback('Failed to create video player'));
  }

  /****************************************************************************
   * ParaView session start callback.
   * @private
   * @param connection The connection configuration object.
   ***************************************************************************/
  function pvStartCallback (connection) {
    pv.connection = connection;
    if(connection.error) {
      alert(connection.error);
      window.close(); /// @todo don't kill browser on error
      return;
    }

    console.log('start: connecting session');
    pvConnectSession();
  }

  /****************************************************************************
   * ParaView session start error callback. If an error occurs trying to start
   * the session, retry with the embedded URL.
   * @private
   ***************************************************************************/
  function pvRetryCallback () {
    console.log(
      "The remote session did not properly start. Trying embedded URL.");

    pv.connection = {
      sessionURL: "ws://" + location.hostname + ":" + location.port + "/ws"
    };
    console.log('connect session ', this, pv);
    pvConnectSession();
  }

  /****************************************************************************
   * Connect ParaView session.
   * @private
   ***************************************************************************/
  function pvConnectSession () {
    if(location.protocol == "http:") {
      pv.connection.sessionURL =
        pv.connection.sessionURL.replace("wss:", "ws:");
    }

    setBusy(true);
    console.log('connect session ', this, pv);
    paraview.connect(pv.connection, pvConnectCallback, pvErrorCallback);
  }

  /****************************************************************************
   * Video player create callback. This stores the handle of the successfully
   * created video player.
   * @private
   * @param vpId The id of the video player.
   ***************************************************************************/
  function vpCreateCallback (vpId) {
    videoPlayerId = vpId;
  }

  /****************************************************************************
   * Video player set data callback.
   * @private
   * @param {string} err Error message, if an error occurred.
   ***************************************************************************/
  function vpSetVideoDataCallback (err) {
    if (err !== null) {
      console.log('Failed to set video data: ', err);
    }

    pv.viewport.render();
  }

  /****************************************************************************
   * Format a date string from Unix time.
   * @private
   * @param {integer} microseconds Number of microseconds since epoch.
   * @param {boolean} longForm Whether to return long or short form.
   ***************************************************************************/
  function formatDate (microseconds, longForm) {
    if (!microseconds) {
        return '-';
    }
    var date = new Date(microseconds / 1000);
    var dayStr = date.getFullYear() + '-' +
                ('0' + (date.getMonth()+1)).slice(-2) + '-' +
                ('0' + date.getDate()).slice(-2);
    var timeStr = ('0' + date.getHours()).slice(-2) + ':' +
                  ('0' + date.getMinutes()).slice(-2) + ':' +
                  ('0' + date.getSeconds()).slice(-2);
    if (longForm) {
      var subsecStr = date.getMilliseconds() + (microseconds % 1000);
      return dayStr + ' ' + timeStr + '.' + subsecStr;
    } else {
      return timeStr;
    }
  }

  /****************************************************************************
   * Window resize callback. Refreshes the render view from the server.
   * @private
   ***************************************************************************/
  function windowResizeCallback () {
    if(pv.viewport) {
      pv.viewport.render();
    }
    // Resize the Google Maps viewport to fit the window height
    var winHeight = $(window).height();
    $('#map-canvas').height(winHeight - 90);

    var activeTab = $('.query-results .tab-pane.active');

    // Set max height on the query results view so it stays in the window
    var desiredHeight = winHeight - activeTab.offset().top - 70;
    var h = Math.max(desiredHeight, 80) + 'px';
    $('#active-results').css('max-height', h);
    $('#active-feedback-requests').css('max-height', h);
  }

  /****************************************************************************
   * Make specified result 'selected'. This is called as a callback when a
   * result tr element is clicked.
   * @private
   ***************************************************************************/
  function selectResult (event) {
    var $selectedRow = $(this);

    if ($selectedRow.hasClass('highlight')) {
      undoSelectResult();
      event.stopPropagation();
      return;
    }

    $('div.visgui-result-row.highlight').removeClass('highlight');
    $selectedRow.addClass('highlight');

    var iid = parseInt($selectedRow.attr('result-instance-id'), 10);

    currentStartTime = parseInt($selectedRow.attr('start-time'), 10);
    currentEndTime = parseInt($selectedRow.attr('end-time'), 10);
    $('div#video-scrubber').slider('option', {
                              disabled: false,
                              min: currentStartTime,
                              max: currentEndTime
                            });
    updateTimestampDisplay(currentStartTime);

    undoHighlightMarkers();
    highlightMarker(markerCollection[iid]);

    that.displayTracksMetadata(tracksMetadataMap[iid]);
    pv.session.call("pv:setVideoData", videoPlayerId, iid).then(
      vpSetVideoDataCallback,
      createErrorCallback('Failed to set video data'));

    event.stopPropagation();
  }

  /****************************************************************************
   * Undo mark selection action previously performed.
   * @private
   ***************************************************************************/
  function undoSelectResult() {
    $('div.visgui-result-row.highlight').removeClass('highlight');
    for (markerId in markerCollection) {
      if (markerCollection.hasOwnProperty(markerId)) {
        undoHighlightMarker(markerCollection[markerId]);
      }
    }
   }

  /****************************************************************************
   * Get the css color for a result swatch. For now, simply colors by score.
   * @param result The result object.
   * @private
   ***************************************************************************/
  function getResultColor (result) {
    for (var i = 0; i < colorStops.length; i++) {
      if (result.relevancyScore > colorStops[i].value) {
        return colorStops[i].color;
      }
    }
    return 'gray';
  }

  /****************************************************************************
   * Call this whenever the video timestamp changes. This will update the
   * relevant view components accordingly.
   * @param timestamp The raw timestamp value.
   * @private
   ***************************************************************************/
  function updateTimestampDisplay (timestamp) {
    if (timestamp >= currentStartTime && timestamp <= currentEndTime) {
      $('div#video-scrubber').slider('option', 'value', timestamp);
      $('div.video-timestamp').text(formatDate(timestamp));
    }
  }

  /****************************************************************************
   * Update the video render view.
   * @private
   ***************************************************************************/
  function render () {
    pv.session.call("pv:render");
    pv.viewport.render();
  }

  /****************************************************************************
   * Show a generic indeterminate loading dialog.
   * @param message The message to display next to the loading icon.
   * @private
   ***************************************************************************/
  function showLoadingDialog (message) {
    $('div#loading-dialog').modal().find('span.loading-message').text(message);
  }

  function hideLoadingDialog () {
    $('div#loading-dialog').modal('hide');
  }

  /****************************************************************************
   * Video play callback.
   * @timestamp The current timestamp of the video.
   * @private
   ***************************************************************************/
  function vpPlayCallback (timestamp) {
    updateTimestampDisplay(timestamp);
    render();
    if (shouldRunAnimation) {
      timeoutInstance = setTimeout(vpAnimateCallback, 1);
    }
  }

  /****************************************************************************
   * Video stop callback.
   * @private
   ***************************************************************************/
  function vpStopCallback () {
    timeoutInstance = null;
  }

  /****************************************************************************
   * Video animation ('tick') callback.
   * @private
   ***************************************************************************/
  function vpAnimateCallback () {
    pv.session.call("pv:videoPlayer:play", videoPlayerId).then(
      vpPlayCallback, createErrorCallback('Failed to play video'));
  }

  /****************************************************************************
   * Stop the video playback.
   * @private
   ***************************************************************************/
  function stopVideo () {
    pv.session.call("pv:videoPlayer:stop", videoPlayerId).then(
      vpStopCallback, createErrorCallback('Failed to stop video playback'));
  }

  /****************************************************************************
   * Callback from the video scrubber slider being changed. Note that this
   * is called in both the case of programmatic value updates as well as
   * manual user-controlled changes.  We want to ignore the programmatic ones
   * and only update the video when manual changes are made.
   * @param event The update event
   * @param ui The slider widget object.
   * @private
   ***************************************************************************/
  function videoSliderChanged (event, ui) {
    if (event.originalEvent) { // test if this is a manual slider change
      var pos = parseInt(ui.value, 10);
      pv.session.call("pv:videoPlayer:seek", videoPlayerId, pos).then(
        vpPlayCallback, createErrorCallback('Failed to seek video'));
    }
  }

  /****************************************************************************
   * Set a rating on a result for IQR.
   * @param row The jQuerified result row DOM element.
   * @param {int} value IqrClassification enum value.
   * @private
   ***************************************************************************/
  function setRating (row, value) {
    var iid = parseInt(row.attr('result-instance-id')),
        rows = $('.visgui-result-row[result-instance-id=' + iid + ']');

    var cols = rows.find('.col-number');

    if (value == IqrClassification.PositiveExample) {
      cols.removeClass('not-relevant').addClass('relevant');
    }
    else if (value == IqrClassification.NegativeExample) {
      cols.removeClass('relevant').addClass('not-relevant');
    }
    else {
      cols.removeClass('relevant not-relevant');
    }
    pv.session.call("pv:query:setResultFeedback", iid, value).then(
      noop, createErrorCallback('Failed to set result feedback'));
  }

  // Initial page loading behavior should go here.
  $(document).ready(function () {
    $('div#video-scrubber').slider({
      disabled: true,
      change: videoSliderChanged
    });

    // Bind these actions here since the context menu is a singleton
    $('#result-context-menu .rate-relevant-action').click(function () {
      setRating(currentRow, IqrClassification.PositiveExample);
    });
    $('#result-context-menu .rate-not-relevant-action').click(function () {
      setRating(currentRow, IqrClassification.NegativeExample);
    });
    $('#result-context-menu .remove-rating-action').click(function () {
      setRating(currentRow, IqrClassification.UnclassifiedExample);
    });
  });

  /****************************************************************************
   * Initialize openstreetmap view.
   * @private
   ***************************************************************************/
  function initializeOsmMap (nodeId, mapConfig) {
    var layerConfigs = mapConfig["osm_map_layers"] || [];

    map = new OpenLayers.Map(nodeId);
    var layer = null, i = 0;
    for (i = 0; i < layerConfigs.length; ++i) {
      layer = layerConfigs[i];
      var layerType = layer["type"] || "OSM";
      if (layerType == "OSM") {
        map.addLayer(
          new OpenLayers.Layer.OSM(layer["name"], layer["urls"]));
        // Spherical Mercator Projection
        toProjection = new OpenLayers.Projection("EPSG:900913")
      }
      else if (layerType == "WMS") {
        map.addLayer(
          new OpenLayers.Layer.WMS(layer["name"], layer["url"],
                                   layer["params"]));
        // WGS 1984
        toProjection = new OpenLayers.Projection("EPSG:4326")
      }
      else {
        console.log('unknown map layer type ' + layerType);
      }
    }

    map.zoomToMaxExtent();
  }

  /****************************************************************************
   * Fade-out a marker.
   * @param marker Marker instance.
   * @param value {number} Opacity [0-1] for the marker. Default is 0.2.
   * @private
   ***************************************************************************/
  function fadeOutMarker(marker, value) {
    if (!marker) {
      return;
    }

    // Default is 0.2
    var val = 0.2;
    if (typeof value !== 'undefined' && value !== null) {
      val = value;
    }
    marker.setOpacity(val);
  }

  /****************************************************************************
   * Fade-out markers.
   * @param value {number} Opacity [0-1] for the markers.
   * @private
   ***************************************************************************/
  function fadeOutMarkers(value) {
    var markerId = null;
    for (markerId in markerCollection) {
      if (markerCollection.hasOwnProperty(markerId)) {
        fadeOutMarker(markerCollection[markerId], value);
      }
    }
  }

  /****************************************************************************
   * Fade-in a marker.
   * @param marker Marker instance.
   * @param value {number} Opacity [0-1] for the marker. Default is 1.0.
   * @private
   ***************************************************************************/
  function fadeInMarker(marker, value) {
    if (!marker) {
      return;
    }
    var val = 1.0;
    if (typeof value !== 'undefined' && value !== null) {
      val = value;
    }
    marker.setOpacity(val);
  }

  /****************************************************************************
   * Fade-in markers.
   * @param value {number} Opacity [0-1] for the markers.
   * @private
   ***************************************************************************/
  function fadeInMarkers(value) {
    var markerId = null;
    for (markerId in markerCollection) {
      if (markerCollection.hasOwnProperty(markerId)) {
        fadeInMarker(markerCollection[markerId], value);
      }
    }
  }

  /****************************************************************************
   * Highlight selected marker.
   * @param marker Marker instance.
   * @private
   ***************************************************************************/
  function highlightMarker (marker) {
    fadeOutMarkers(0.0);
    if (marker) {
      marker.setUrl('/images/icons/marker-gold.png');
      fadeInMarker(marker, 1.0);
    }
  }

  /****************************************************************************
   * Undo highlight marker.
   * @param marker Marker instance.
   * @private
   ***************************************************************************/
  function undoHighlightMarker (marker) {
    if (marker) {
      marker.setUrl('/images/icons/marker-blue.png');
      fadeInMarker(marker, 1.0);
    }
  }

  /****************************************************************************
   * Undo highlight for all markers.
   * @param value Opacity [0-1] for the markers.
   * @private
   ***************************************************************************/
  function undoHighlightMarkers (value) {
    var markerId = null;
    for (markerId in markerCollection) {
      if (markerCollection.hasOwnProperty(markerId)) {
        undoHighlightMarker(markerCollection[markerId], value);
      }
    }
  }

  /****************************************************************************
   * Clear cached data for visualization purpose.
   * @private
   ***************************************************************************/
  function clearCache () {
    markerCollection = {};
  }

  /****************************************************************************
   * Helper function to create the DOM element for each result row.
   * @param result The result object.
   * @param row The DOM element to instantiate.
   * @private
   ***************************************************************************/
  function makeResultRow (result, row, scoreField) {
    scoreField = scoreField || 'relevancyScore';
    row.addClass('visgui-result-row')
       .attr('result-instance-id', result.instanceId)
       .attr('start-time', result.startTime)
       .attr('end-time', result.endTime);

    row.find('.col-number').html(result.instanceId);
    row.find('.col-rank').html(result.rank);
    row.find('.col-score').html(result[scoreField].toFixed(3)).tooltip({
      title: result[scoreField],
      animation: false
    });
    row.find('.col-start-time').html(formatDate(result.startTime)).tooltip({
      title: formatDate(result.startTime, true),
      animation: false
    });
    row.find('.col-end-time').html(formatDate(result.endTime)).tooltip({
      title: formatDate(result.endTime, true),
      animation: false
    });
    row.find('.visgui-color-swatch').css('background-color',
                                          getResultColor(result));

    if (result.userScore == IqrClassification.PositiveExample) {
      row.find('.col-number').addClass('relevant');
    }
    else if (result.userScore == IqrClassification.NegativeExample) {
      row.find('.col-number').addClass('not-relevant');
    }

    // Add ability to select results, fade them in, and add context menu
    row.click(selectResult)
       .fadeIn('slow')
       .contextmenu({ // put this last as $.fn.contextmenu() doesn't chain
         target: '#result-context-menu',
         before: function (e, target) {
           currentRow = target;
           return true;
         }
       });

    return row;
  }

  /**************************************************************************
   * Add result markers layer to the map.
   *************************************************************************/
  function addResultMarkersLayer() {
    markers = new OpenLayers.Layer.Markers("Markers");
    map.addLayer(markers);
  }

  /**************************************************************************
   * Remove result markers layer from the map.
   *************************************************************************/
  function removeResultMarkersLayer() {
    if (markers) {
      markers.destroy();
      markers = null;
    }
  }

  return {
    /**************************************************************************
     * Return a key-value store containing the configuration option contained
     * in the inputfile.
     *************************************************************************/
    parseConfig : function (configUrl, callback) {
      $.get(configUrl, function (data) {
        if ( !data || data === "") {
          // error
          return;
        }

        var json;
        try {
          json = jQuery.parseJSON(data);
        }
        catch (e) {
          console.log("error parsing configuration: " + e)
          return;
        }
        callback(json || {});
      }, "text");
    },

    /**************************************************************************
     * Start a paraview session.
     *************************************************************************/
    startSession: function () {
      if(!$('body').hasClass("initialized")) {
        $('body').addClass("initialized");

        setBusy(true);
        console.log('starting session');
        paraview.start(config, pvStartCallback, pvRetryCallback);
      }
    },

    /**************************************************************************
     * Initialize view.
     *************************************************************************/
    initializeView: function (mapConfig) {
      // Bind events to actions
      $('#results').click(vgweb.undoSelectResult);

      // Make the viewport be full screen
      $(window).resize(windowResizeCallback).trigger('resize');

      // Default is openstreetmap
      initializeOsmMap('map-canvas', mapConfig);
    },

    /**************************************************************************
     * Force a refresh of the ParaView render window on the client.
     *************************************************************************/
    updateView: function () {
      if(pv.viewport) {
        pv.viewport.invalidateScene();
      }
    },

    /**************************************************************************
     * Close ParaView session. This should be called when the application is
     * about to close.
     *************************************************************************/
    endSession: function () {
      if(pv.session) {
        pv.viewport.unbind();
        paraview.stop(pv.connection);
        pv = {};
      }
    },

    /**************************************************************************
     * Call this when a query file has been chosen. The chosen file
     * must reside in a file input element with id "query-file-input".
     *************************************************************************/
    querySelected: function () {
      var file = $('#query-file-input').get(0).files[0];
      if (file) {
        this.uploadAndExecuteQuery();
      }
    },

    /**************************************************************************
     * IQR: refine results.
     *************************************************************************/
    refineResults: function () {
      that = this;

      showLoadingDialog('Refining results...');
      pv.session.call("pv:query:refine").then(function (okay) {
        if (okay) {
          refreshIntervalIds.push(
            setInterval(function() { that.checkIfCompleted(); }, 1000));
        } else {
          console.log('Failed to refine query');
          hideLoadingDialog();
        }
      },
      function (err) {
        hideLoadingDialog();
        createErrorCallback('Failed to refine query')(err);
      });
    },

    /**************************************************************************
     * This should be called after a query has been selected in the
     * $('#query-input-file') input element. Uploads and executes the
     * selected query.
     *************************************************************************/
    uploadAndExecuteQuery: function () {
      var file = $('#query-file-input').get(0).files[0];
      var reader = new FileReader();
      that = this;

      reader.onload = function (e) { that.executeQuery(e.target.result); };
      reader.readAsText(file);
    },

    /**************************************************************************
     * Execute a query from a file loaded previously.
     * @param queryPlan
     *************************************************************************/
    executeQuery: function (queryPlan) {
      that = this;
      pv.session.call("pv:getQueryUrl").then(function (queryUrl) {
        if (queryUrl === '') {
          console.log('invalid query url');
          return;
        }
        pv.session.call("pv:executeQuery", queryUrl, queryPlan, 1500).then(
          function(reply) {
            if (reply === false) {
              // Query failed to execute
              pv.session.call("pv:query:getStatus").then(function(status) {
                console.log('Failed to execute query: ' + status);
              });
              return;
            }
            // Query is executing; wait for it to finish
            showLoadingDialog('Executing query...');
            refreshIntervalIds.push(
              setInterval(function() { that.checkIfCompleted(); }, 1000));
          }, function (error) {
            hideLoadingDialog();
            createErrorCallback('Failed to execute query')(error);
          });
      });
    },

    /**************************************************************************
     * Check if the last executed query is completed, and display the results
     * if it is.
     *************************************************************************/
    checkIfCompleted: function () {
      that = this;
      var i = 0;
      pv.session.call("pv:query:isCompleted").then(function(reply) {
        if (reply === true) {
          for (i = 0; i < refreshIntervalIds.length; ++i) {
            clearInterval(refreshIntervalIds[i]);
          }
          pv.session.call("pv:query:getAllResults", 0, 100).then(
            function (resp) {
              hideLoadingDialog();
              that.displayResults(resp.resultIds, resp.feedbackIds,
                                  resp.results);
            },
            createErrorCallback('Failed to retrieve results'));
        }
      });
    },

    /**************************************************************************
     * Display results returned by query execution.
     * @param results The serialized result set
     * @param feedbackIds The iid's of results with feedback requests
     *************************************************************************/
    displayResults: function (resultIds, feedbackIds, results) {
      that = this;
      var activeResults = $("#active-results"),
          feedbackRequests = $("#active-feedback-requests"),
          data = {},
          center = [0.0, 0.0],
          locationCount = 0;

      // Clear any previous results
      activeResults.find('div.visgui-result-row').remove();
      feedbackRequests.find('div.visgui-result-row').remove();
      tracksMetadataMap = {};

      // Clear cache
      clearCache();

      // Remove layers that are no longer valid
      removeResultMarkersLayer();

      // Iterate through all results and display them on the map
      $.each(results, function(idx, result) {
        tracksMetadataMap[result.instanceId] = result.tracks;

        // Draw locations on the map
        if (result.location) {
          if (!markers) {
            addResultMarkersLayer();
          }
          lat = result.location.northing;
          lon = result.location.easting;

          center[0] += lat;
          center[1] += lon;
          locationCount++;

          var size = new OpenLayers.Size(21, 25);
          var offset = new OpenLayers.Pixel(-(size.w/2), -size.h);
          var icon = new OpenLayers.Icon('/images/icons/marker-blue.png',
                                         size, offset);
          var xf = new OpenLayers.LonLat(lon, lat).transform(fromProjection,
                                                             toProjection)
          var marker = new OpenLayers.Marker(xf, icon)
          markers.addMarker(marker);
          markerCollection[result.instanceId] =  marker;
        }
      });

      // Iterate through sorted result ID list and add to the result list
      sortResults(resultIds, results);
      $.each(resultIds, function(idx, iid) {
        var row = $('div#template-result-row').clone().removeAttr('id'),
            result = results[iid];
        activeResults.append(makeResultRow(result, row));
      });

      // Iterate through sorted feedback request IDs and add to the feedback
      // requests list
      sortResults(feedbackIds, results,
                  ['preferenceScore', 'instanceId'], [true, false]);
      $.each(feedbackIds, function(idx, iid) {
        var row = $('div#template-feedback-row').clone().removeAttr('id'),
            result = results[iid];
        feedbackRequests.append(makeResultRow(result, row, 'preferenceScore'));
      });

      // Compute center location of results
      if (locationCount > 0) {
        center[0] /= locationCount;
        center[1] /= locationCount;

        // Set new center and zoom for map
        map.setCenter(new OpenLayers.LonLat(center[1], center[0]).transform(
          fromProjection, toProjection), 10, false, false);
      }
    },

    /**************************************************************************
     * Display tracks meta data.
     * @param tracksMetadata
     *************************************************************************/
    displayTracksMetadata: function (tracksMetadata) {
      var tableVideoMetadata = $('#table-videometadata'),
          i = null;

      // Clear previous display
      $('#table-videometadata tbody tr').remove();

      for (i = 0; i < tracksMetadata.length; ++i) {
        var row = $(document.createElement('tr'));
        row.append('<td><label class="checkbox">' +
                   '<input type="checkbox">' +
                   'Track-' + tracksMetadata[i].id.serialNumber +
                   '</label></td>');
        row.append('<td>' + tracksMetadata[i].id.source + '</td>');
        row.append('<td>' + '' + '</td>');
        row.append('<td>' + '' + '</td>');
        tableVideoMetadata.append(row);
      }
    },

    /**************************************************************************
     * Toggle display of trails in video.
     *************************************************************************/
    toggleTrails: function () {
      // Get state; it is opposite because the change hasn't been applied yet
      // when we are called
      var state = !$('#toggle_trails').hasClass('active');
      pv.session.call("pv:videoPlayer:setTrailsVisible",
                      videoPlayerId, state).then(
        render, createErrorCallback('Failed to set trail visibility'));
    },

    /**************************************************************************
     * Undo all selections
     *************************************************************************/
    undoSelectResult: function () {
      undoSelectResult();
    },

    /**************************************************************************
     * Begin video playback. If video is running already, restarts it.
     *************************************************************************/
    runAnimation: function () {
      if (timeoutInstance) {
        clearTimeout(timeoutInstance);
        timeoutInstance = null;
        pv.session.call("pv:videoPlayer:stop", videoPlayerId).then(
          this.runAnimation,
          createErrorCallback('Failed to restart video playback'));
      }
      else {
        shouldRunAnimation = true;
        timeoutInstance = setTimeout(vpAnimateCallback, 1);
      }
    },

    /**************************************************************************
     * Stop video playback.
     *************************************************************************/
    stopAnimation: function () {
      shouldRunAnimation = false;
      clearTimeout(timeoutInstance);
      stopVideo();
    }
  };
};
