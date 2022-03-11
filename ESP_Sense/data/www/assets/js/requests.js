//success(data)
//error(request, status, error)
function requestJsonData(url, success, error) {
    $.ajax({
        dataType: "json",
        type: 'GET',
        url: url,
        data: null,
        success: success,
        error: error
    });
}
