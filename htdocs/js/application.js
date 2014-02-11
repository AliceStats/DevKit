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
                type: 'error',
                align: 'center',
                width: 'auto',
                allow_dismiss: false
            });
        }
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
        alert($(this).attr("data-replay"));
    });

    // add the about tab
    addTab("About", $("#about-view").html());

    // get a list of loadable replays
    request("00", "", function(data) {
        $.each(data.data, function(index, value) {
            $("#openmenu").append('<li><a class="openreplay" data-replay="'+value+'" href="#">'+value+'</a></li>');
        });
    });
});