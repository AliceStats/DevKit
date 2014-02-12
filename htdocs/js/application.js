var composeCount = 0;   // number of tabs added to switch to the last created one

// shows a specific tab
function switchToTab(tabId) {
    $('#tabs a[href="#' + tabId + '"]').tab('show');
}

// adds a new tab containing the specified html
function addTab(title, content, classes, addon) {
    $("#content").show();
    var tabId = "tab" + composeCount++;
    $('#tabs .nav-pills').append('<li><a href="#' + tabId + '"><button class="close closeTab" type="button" >&times;</button> '+title+' </a></li>');
    $('#tabs .tab-content').append('<div class="tab-pane '+classes+'" data-addon="'+addon+'" id="' + tabId + '">'+content+'</div>');
    switchToTab(tabId);
}

// does an api request
function request(type, args, callback) {
    $.ajax({
        url: "/api/"+type+"/"+args,
        dataType: "json"
    }).done(function(data) {
        if (data.success) {
            callback(data);
        } else {
            $.bootstrapGrowl(data.data, {
                type: 'info',
                align: 'center',
                width: 'auto',
                allow_dismiss: false
            });
        }
    });
}

// initialize page
$(function () {
    // --- Templates ---

    var entityTpl = Handlebars.compile($("#entity-template").html());

    // --- Functions that require templates ---

    // refresh content
    function refresh() {
        // refresh list of replays
        request("00", "", function(data) {
            $("#openmenu").html("");

            $.each(data.data, function(index, value) {
                $("#openmenu").append('<li><a class="openreplay" data-replay="'+value+'" href="#">'+value+'</a></li>');
            });
        });

        // refresh the list of stringtables
        request("04", "", function(data) {
            $("#stringtable-list").html(""); // reset
            $.each(data.data, function(name, subs) {
                $("#stringtable-list").append('<optgroup id="group-'+name+'" label="'+name+'"></optgroup>');
                $.each(subs, function(id, sub){
                     $("#stringtable-list").append('<option data-group="'+name+'" data-filter="'+sub.toLowerCase()+'">'+sub+'</option>');
                });
            });
        });

        // refresh the list of entities
        request("06", "", function(data) {
            data.data.sort(function(a, b) { return a[1] > b[1] ? 1 : -1; });
            $("#entity-list").html("");

            $.each(data.data, function(id, val) {
                $("#entity-list").append(
                    '<option data-filter="'+val[1].toLowerCase()+'" value="'+val[0]+'">'+val[1]+' - ('+val[0]+')</option>'
                );
            });
        });

        // refresh loaded entities
        $( ".entity-tab" ).each(function() {
            var id = $(this).attr("data-addon");
            var tab = $(this);
            request("07", id, function (data) {
                tab.html(entityTpl(data));
            });
        });

        // refresh status
        request("08", "", function(data) {
            $("#openpath").val(data.data.file);
            $("#status-tick").val(data.data.ticks);
            $("#status-score").val(data.data.score);
            $("#status-time").val(data.data.time);
        });
    }

    // --- Filter ----

    $(".filter").on('input', function() {
        var target = "#"+$(this).attr("data-target")+" option";
        var filter = $(this).val().toLowerCase();

        // Still slow for stringtables,
        // TODO: Test this on older hardware to see if this is a problem

        $(target).filter(function () {
            if ($(this).attr("data-filter").indexOf(filter) == -1) {
                if (!$(this).parent().hasClass("hidden")) {
                    if (!$(this).parent().find(".hidden").length) {
                        $(this).parent().append('<span class="hidden"></span>');
                    }

                    // hide
                    $(this).appendTo($(this).parent().find(".hidden"));
                }
            } else {
                if (filter === "" || $(this).parent().hasClass("hidden")) {
                    var group = $(this).attr("data-group");
                    if (!group)
                        $(this).appendTo($(this).parent().parent());
                    else
                        $(this).insertAfter($("#group-"+group));
                }
            }
        });

    });

    // --- Tabbing ----

    // register onClose for all tabs
    $("#tabs").on("click", ".close", function () {
        var tabContentId = $(this).parent().attr("href");
        $(this).parent().parent().remove(); //remove li of tab
        $(tabContentId).remove(); //remove respective tab content

        if ($("#tabs .tab-content").html() === "") {
            $("#content").hide();
        }
    });

    // register onClick for all tabs
    $("#tabs").on("click", "a", function (e) {
        e.preventDefault();
        $(this).tab('show');
    });

    // --- Control Panel ---

    // toggle control board
    $("#toggle-control").click(function() {
        $("#control").toggle();
    });

    // add about tab
    $("#main-about").click(function() {
        addTab("About", $("#about-view").html());
    });

    // listen on replay open
    $("#openmenu").on("click", ".openreplay", function() {
        var attr = $(this).attr("data-replay");
        request("01", attr, function(data) {
            $("#openpath").val(attr);
            $.bootstrapGrowl(data.data);
            refresh();
        });
    });

    // listen on replay parse
    $("#btnparse").click(function() {
        request("02", $("#parsecount").val(), function(data) {
            $.bootstrapGrowl(data.data);
            refresh();
        });
    });

    // listen on replay reload
    $("#btnreload").click(function() {
        var attr = $("#openmenu").val();
        request("01", attr, function(data) {
            $.bootstrapGrowl(data.data);
            refresh();
        });
    });

    // listen on replay close
    $("#btnclose").click(function() {
        request("03", "", function(data) {
            $("#openpath").val("");
            $.bootstrapGrowl(data.data);
            refresh();
        });
    });

    // --- Overlays ---

    // listen on stringtable load
    $("#stringtable-load").click(function() {

    });

    // listen on entity load
    $("#entity-load").click(function() {
        // add a tab for each selected entity
        $( "#entity-list option:selected" ).each(function() {
            var id = $(this).val();
            var name = $(this).html();
            request("07", id, function (data) {
                addTab("Entity "+name, entityTpl(data), "entity-tab", id);
            });
        });
    });

    // --- Setup ---

    // hide the control panel by default
    //$("#control").toggle();

    // add the about tab
    //addTab("About", $("#about-view").html());

    // refresh content
    refresh();
});