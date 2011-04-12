/// Global variable for the summary content.
var test_results = {};

/**
 * Digest the summary JSON object, filling the test_results variable.
 * @param summary object containing the summary data (response of AJAJ request).
 */
function parseSummary(summary) {
	// Prepare the counters
	test_results.counter = {"PASS":     0,
			                "FAIL":     0,
			                "UNTESTED": 0,
			                "ERROR":    0,
			                "total":    summary.length};
	test_results.tests = {};
	test_results.not_passed = [];
	test_results.all_tests = [];
	for (var i in summary) {
		var test = summary[i];
		// Update the counter of the outcome.
		++test_results.counter[test.outcome];
		//var test_path = split(".", test.id);
		test_results.tests[test.id] = test;
		test_results.all_tests.push(test.id);
		if (test.outcome != "PASS") {
			test_results.not_passed.push(test.id);
		}
	}
}

/**
 * Set the html of the selected elements to a spinning icon (for visual feed back).
 * Note: the image has been downloaded from http://www.ajaxload.info
 */
jQuery.fn.loadingIcon = function() {
	return this.html('<img src="ajax-loader.gif"/>');
}

/**
 * Load the annotations file (annotations.json) and fill the annotations block of
 * the DOM.
 */
jQuery.fn.annotations = function(data) {
	// Collect the list of keys.
	// These keys are special and must appear first in the list.
	var keys = ["qmtest.run.start_time", "qmtest.run.end_time"];
	// Add all the other keys to the list of keys.
	for (var key in data) {
		if ($.inArray(key, keys) == -1) {
			keys.push(key);
		}
	}
	// Prepare a table for the annotations
	var tbody = $("<tbody/>");
	// Add the rows of the table
	for (var i in keys) {
		var key = keys[i];
		var value = data[key];
		if ($.isArray(value)) {
			// In case the value is an Array, let's format it as a list
			var tmp = $('<ul/>');
			for (i in value) {
				tmp.append($('<li/>').text(value[i]))
			}
			value = tmp;
		}
		tbody.append($("<tr/>")
				.append($('<td class="key"/>').text(key))
				.append($('<td class="value"/>').html(value)));
	}
	// Insert the code in the annotations block and enable the toggle button
	return this.html($('<table/>').append(tbody));
}

/// Display the summary table
jQuery.fn.summary = function() {
	var counter = test_results.counter;
	// Prepare a table layout (like the one produced usually by QMTest).
	var tbody = $("<tbody/>");
	// row with the total
	tbody.append($("<tr/>")
		 .append($("<td align='right'/>").text(counter.total))
		 .append($("<td colspan='3'/><td>tests total</td>")));
	var result_types = ["FAIL", "ERROR", "UNTESTED", "PASS"];
	for (var i in result_types) {
		var result_type = result_types[i];
		if (counter[result_type]) {
			tbody.append($("<tr/>")
				     .append($("<td align='right'/>")
					     .text(counter[result_type]))
				     .append("<td>(</td>")
				     .append($("<td align='right'/>")
					     .text(Math.round(counter[result_type]
							      / counter.total * 100)))
				     .append("<td>%)</td>")
				     .append($("<td/>")
					     .text("tests " + result_type))
				     .addClass(result_type));
		}
	}
	return this.html($("<table/>").html(tbody));
}

/** Generate foldable lists.
 */
jQuery.fn.foldable = function() {
    this.each(function() {
	    var me = $(this);
	    me.addClass("folded")
	    // wrap the content of the element with a clickable span
	    // (includes the ul)
	    .wrapInner($("<span class='clickable'/>")
		       .click(function(){
			       var me = $(this);
			       me.next().toggle();
			       me.parent().toggleClass("folded expanded");
			       return false; // avoid bubbling of the event
			   }));
	    // this moves the ul after the span (and hides it in the meanwhile)
	    me.append($("span > ul", me).hide());
	});
};

/** Modify the items that have the "url" data, making them clickable
 *  and followed by a hidden block with the content of the link specified.
 */
jQuery.fn.loader = function() {
    this.each(function() { // loop over all the selected elements
      var me = $(this);
      if (me.data("url")) { // modify the element only if it does have a data "url"
	  me.addClass("folded")
	      // wrap the "text" of the element with a clickable span that loads the url
	      .wrapInner($("<span class='clickable'/>")
			 .click(function(){ // trigger the loading on click
				 var me = $(this);
				 me.after($("<div/>").loadingIcon().load(me.parent().data("url")));
				 me.unbind("click"); // replace the click handler
				 me.click(function(){ // this handler just toggle the visibility
					 var me = $(this);
					 me.next().toggle();
					 me.parent().toggleClass("folded expanded");
					 return false; // avoid bubbling of the event
				     });
				 me.parent().toggleClass("folded expanded");
				 return false; // avoid bubbling of the event
			     }));
      }
    });
};

/**
 * Helper function to re-use the code for the toggle button callback
 */
jQuery.fn.toggleNextButton = function(data) {
	if (data === undefined) data = {};
	if (data.hide === undefined) data.hide = "(hide)";
	if (data.show === undefined) data.show = "(show)";
	if (data.start_visible === undefined) data.start_visible = false;
	this.click(function() {
		var me = $(this);
		me.next().toggle();
		if (me.next().is(":visible")) {
			me.text(data.hide);
		} else {
			me.text(data.show);
		}
	}).text(data.start_visible ? data.hide : data.show)
	  .next().toggle(data.start_visible);
	return this;
}

/** Make a given element toggleable using a show/hide button inserted just before
 *  it.
 */
jQuery.fn.makeToggleable = function(data) {
	this.each(function() {
		var btn = $("<span class='clickable'/>");
		$(this).before(btn);
		btn.toggleNextButton(data);
	});
	return this;
}

/// Display the list of results
jQuery.fn.results = function(tests) {
    if (tests === undefined) tests = test_results.not_passed;

    var ul = $("<ul/>");
    for (var i in tests) {
	var test = test_results.tests[tests[i]];

	var entry = $("<li/>")
	    .append($("<span class='testid'/>").text(test.id + ": "))
	    .append($("<span/>").addClass(test.outcome).text(test.outcome));
	if (test.cause) {
	    entry.append(" ")
		.append($("<span class='cause'/>")
			.text(test.cause));
	}
	var fields = $("<ul class='fieldid'/>");
	for (var j in test.fields) {
	    fields.append($("<li/>").data("url", test.id + "/" + test.fields[j])
			  .text(test.fields[j]));
	}
	entry.append(fields);
	ul.append(entry);
    }

    $("li", ul).loader();
    $("li:has(ul)", ul).foldable();

    return this.html(ul)
    	.append($('<div class="clickable">Collapse all</div>').click(function(){
    		$(this).prev().find(".expanded > .clickable")
    			.next().hide()
    			.parent().toggleClass("folded expanded");
    	}));
}

/// Code executed when the page is ready.
$(function () {
	$(".loading").loadingIcon().removeClass("loading");
	// load the summary
	$.get("summary.json", parseSummary, 'json')
	.success(function(){
		$("#summary").summary();
		$("#results").results();
		$("#all_results").html($('<span class="clickable">(show)</span>')
				.click(function(){
					var parent = $(this).parent();
					parent.loadingIcon();
					// actually we already got the JSON data, but this allows to trigger a
					// background execution.
					$.getJSON("summary.json")
					.success(function() {
						parent.results(test_results.all_tests)
							  .makeToggleable({start_visible: true});
					});
				})).removeClass("loading");
	});
	// load annotations
	$.getJSON('annotations.json')
	.success(function(data) {
		$('#annotations').annotations(data).makeToggleable().removeClass("loading");
	});
});
