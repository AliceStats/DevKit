// global variables
var composeCount = 0;   // number of tabs added to switch to the last created one

// for descriptions see http://swarm-deferred.googlecode.com/svn/trunk/src/public/dt_common.h
var flags = [
    {
        name: "Unsigned",
        description: "Value is an unsigned integer",
        flag: (1<<0)
    }, {
        name: "Coord",
        description: "Float / Vector is treated like a world coordinate",
        flag: (1<<1)
    }, {
        name: "NoScale",
        description: "Float / Vector doesn't scale into range, take value as it is",
        flag: (1<<2)
    }, {
        name: "Rounddown",
        description: "Limit high value to range minues one bit unit",
        flag: (1<<3)
    }, {
        name: "Roundup",
        description: "Limit low value to range minus one bit unit",
        flag: (1<<4)
    }, {
        name: "Normal",
        description: "Only valid for vectors, treat vector like a normal",
        flag: (1<<5)
    }, {
        name: "Exclude",
        description: "This property points at another property to be excluded",
        flag: (1<<6)
    }, {
        name: "XYZE",
        description: "XYZ/Exponent encoding for vectors",
        flag: (1<<7)
    }, {
        name: "InsideArray",
        description: "Property is inside an array",
        flag: (1<<8)
    }, {
        name: "Collapsible",
        description: "Set for tables with an offset of 0 that don't change the pointer",
        flag: (1<<11)
    }, {
        name: "Coord-MP",
        description: "Float /Vector is retared like a world coordinate, multi-player version",
        flag: (1<<12)
    }, {
        name: "Coord Low-Precision",
        description: "Fraction component only gets 3 bits instead of 5",
        flag: (1<<13)
    }, {
        name: "Coord Int",
        description: "Coordinates are rounded to integral boundaries",
        flag: (1<<14)
    }, {
        name: "Cell Coord",
        description: "Cell coordinates that can't be negative, bit count indicate maximum value",
        flag: (1<<15)
    }, {
        name: "Cell Coord Low-Precision",
        description: "Fraction component only gets 3 bits instead of 5",
        flag: (1<<16)
    }, {
        name: "Cell Coord Integral",
        description: "Rounded to integral boundaries",
        flag: (1<<17)
    }, {
        name: "Changes Often",
        description: "Moved to head of sendtable so it gets a small index",
        flag: (1<<18)
    }
];

var modes = [
    "-- Unkown --",
    "All Pick",
    "Captains Mode",
    "Random Draft",
    "Single Draft",
    "All Random",
    "Introduction",
    "Diretide",
    "Reverse Captains Mode",
    "Greeviling",
    "Tutorial",
    "Mid Only",
    "Least Played",
    "New Player Pool",
    "Compendium Match"
];

var states = [
    "-- Unkown --", // none state
    "Loading",
    "Picking",
    "-- Unkown --", // strategy state
    "Pregame",
    "Playing",
    "Postgame"
];

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

    Handlebars.registerHelper('typeToString', function(t) {
        switch (t) {
            case 0:
                return "Int";
            case 1:
                return "Float";
            case 2:
                return "Vector XYZ";
            case 3:
                return "Vector XY";
            case 4:
                return "String";
            case 5:
                return "Array";
            case 6:
                return "DataTable";
            case 7:
                return "Int64";
        }
    });

    Handlebars.registerHelper('flag', function(f) {
        var ret = "";
        $.each(flags, function(index, key) {
            if (f & key.flag)
               ret += '<span class="label label-default" title="'+key.description+'">'+key.name+'</span>&nbsp;';
        });

        return ret;
    });

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
            $("#status-mode").val(modes[data.data.mode]);
            $("#status-state").val(states[data.data.state]);
            $("#status-time").val(data.data.time);

            $("#heroicons").html("");
            $.each(data.data.picks, function(index, key) {
                $("#heroicons").append('<img height="28" src="images/icons/'+key.toLowerCase()+'.png" />');
            });
        });
    }

    // --- Filter ----

    $(".filter").on('input', function() {
        var target = "#"+$(this).attr("data-target")+" option";
        var filter = $(this).val().toLowerCase();

        // unselect hidden props if any
        $("#"+$(this).attr("data-target")).find("option").attr("selected", false);

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
        return false;
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
    $("#control").toggle();

    // add the about tab
    addTab("About", $("#about-view").html());

    // refresh content
    refresh();
});