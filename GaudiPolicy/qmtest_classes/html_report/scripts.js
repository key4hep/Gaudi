/// Global variable for the summary content.
var test_results = {};

/// Digest the summary JSON object, filling the test_results variable.
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
 * Load the annotations file (annotations.json) and fill the annotations block of
 * the DOM.
 */
function loadAnnotations() {
	// prepare the toggling button
	$("#annotations").hide()
	  .before($("<span>(show)</span>")
			  .addClass("togglelink clickable")
			  .click(function(){
				  var me = $(this);
				  me.next().toggle();
				  if (me.html() == "(show)") {
				    me.html("(hide)");
				  } else {
					me.html("(show)");
				  }
			  }));
	// Asynchronous retrieval
	$.get('annotations.json', function(data) {
		// Prepare a table with the annotations
		var tbody = $("<tbody/>");
		// This keys are special and must appear first in the table
		var keys = ["qmtest.run.start_time", "qmtest.run.end_time"];
		// Add all the other keys to the list of keys
		for (var key in data) {
			if ($.inArray(key, keys) == -1) {
				keys.push(key);
			}
		}
		// Add the rows of the table
		var running = false;
		for (var i in keys) {
			var key = keys[i];
			var value = data[key];
			if (!value) { // In case of empty value for a key...
				if (key == "qmtest.run.end_time") {
					// ... if the key is "end_time", it means the tests are running
					value = "RUNNING".italics() +
					        " (the page will be updated every 5s)".small();
					running = polling;
				} else {
					// ... otherwise print a meaningful "None"
					value = "None".italics();
				}
			} else {
				if ($.isArray(value)) {
					// In case the value is an Array, let's format it as a list
					var tmp = $('<ul/>');
					for (i in value) {
						tmp.append($('<li/>').text(value[i]))
					}
					value = tmp;
				}
			}
			tbody.append($("<tr/>")
				.append($('<td/>').addClass("key").text(key))
				.append($('<td/>').addClass("value").html(value)));
		}
		// Insert the code in the annotations block
		$('#annotations').html($('<table/>').append(tbody));
	}, "json");
}

/// Display the summary table
function renderSummary(element) {
	var counter = test_results.counter;
	// Prepare a table layout (like the one produced usually by QMTest).
	var tbody = $("<tbody/>");
	// row with the total
	tbody.append($("<tr/>")
		 .append($("<td/>").attr("align", "right").text(counter.total))
		 .append($("<td/>").attr("colspan", "3"))
		 .append($("<td/>").text("tests total")));
	var result_types = ["FAIL", "ERROR", "UNTESTED", "PASS"];
	for (var i in result_types) {
		var result_type = result_types[i];
		if (counter[result_type]) {
			tbody.append($("<tr/>")
				 .append($("<td/>").attr("align", "right").text(counter[result_type]))
				 .append("<td>(</td>")
				 .append($("<td/>").attr("align", "right").text(Math.round(counter[result_type] / counter.total * 100)))
				 .append("<td>%)</td>")
				 .append($("<td/>").text("tests " + result_type)).addClass(result_type));
		}
	}
	element.html($("<table/>").html(tbody));
}

/// Display the list of results
function renderResults(element, tests) {
	var ul = $("<ul/>");
	if (!tests) {
		tests = test_results.not_passed;
	}
	for (var i in tests) {
		var test = test_results.tests[tests[i]];

		var entry = $("<li/>").addClass("folded")
		  .append($("<span/>").addClass("clickable")
	        .append($("<span/>").addClass("testid").text(test.id)).append(": ")
	        .append($("<span/>").addClass(test.outcome).text(test.outcome))
	        .click(function(){
		      var me = $(this);
			  me.next().toggle();
			  me.parent().toggleClass("folded expanded");
			  return false; // avoid bubbling of the event
		    })
	      );
		if (test.cause) {
			entry.children("span")
			     .append(" ")
			     .append($("<span/>").addClass("cause")
				         .text(test.cause));
		}
		var fields = $("<ul/>").hide();
		for (var j in test.fields) {
			fields.append($("<li/>")
					.addClass("folded")
					.append($("<span/>").addClass("clickable")
					  .data("url", test.id + "/" + test.fields[j])
					  .append($("<span/>").addClass("fieldid")
					    .text(test.fields[j]))
					  .click(function(){
						var me = $(this);
						me.after($("<div/>").load(me.data("url")));
						me.unbind("click");
						me.click(function(){
							var me = $(this);
							me.next().toggle();
							me.parent().toggleClass("folded expanded");
							return false; // avoid bubbling of the event
						});
						me.parent().toggleClass("folded expanded");
						return false; // avoid bubbling of the event
					  })
					));
		}
		entry.append(fields);
		ul.append(entry);
	}
	element.append(ul);
}

/// Code executed when the page is ready.
$(function () {
	$.get("summary.json", function(summary) {
		parseSummary(summary);
		renderSummary($("#summary"));
		renderResults($("#results"));

		$("#all_results").hide()
		  .before($("<span>(show)</span>").addClass("togglelink clickable")
				  .click(function(){
					  var me = $(this);
					  me.unbind("click");
					  var nxt = me.next();
					  renderResults(nxt, test_results.all_tests);
					  nxt.show();
					  me.html("(hide)")
		  			  	.click(function(){
		  			  		var me = $(this);
		  			  		me.next().toggle();
		  			  		if (me.html() == "(show)") {
		  			  			me.html("(hide)");
		  			  		} else {
		  			  			me.html("(show)");
		  			  		}
		  			  	});
				  }));
	}, "json");

	loadAnnotations();
});
