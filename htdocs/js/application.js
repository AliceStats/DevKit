var composeCount = 0;   // number of tabs added to switch to the last created one

// shows a specific tab
function switchToTab(tabId) {
    $('#tabs a[href="#' + tabId + '"]').tab('show');
}

// adds a new tab containing the specified html
function addTab(title, content) {
    $("#content").show();
    var tabId = "tab" + composeCount++;
    $('#tabs .nav-pills').append('<li><a href="#' + tabId + '"><button class="close closeTab" type="button" >&times;</button> '+title+' </a></li>');
    $('#tabs .tab-content').append('<div class="tab-pane" id="' + tabId + '">'+content+'</div>');
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
            $("#stringtable-list").append('<optgroup label="'+name+'">');
            $.each(subs, function(id, sub){
                 $("#stringtable-list").append('<option>'+sub+'</option>');
            });
            $("#stringtable-list").append('</optgroup>');
        });
    });

    // refresh the list of entities
    request("06", "", function(data) {
        data.data.sort(function(a, b) { return a[1] > b[1] ? 1 : -1; });
        $("#entity-list").html("");

        $.each(data.data, function(id, val) {
            $("#entity-list").append('<option value="'+val[0]+'">'+val[1]+' - ('+val[0]+')</option>');
        });
    });
}

// initialize page
$(function () {
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

    // toggle control board
    $("#toggle-control").click(function() {
        $("#control").toggle();
    });

    // add about tab
    $("#main-about").click(function() {
        addTab("About", $("#about-view").html());
    });

    // hide the control panel by default
    $("#control").toggle();

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

    // add the about tab
    addTab("About", $("#about-view").html());

    // refresh content
    refresh();
});