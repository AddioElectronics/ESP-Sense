

//success(data)
//error(xhr, status, error)
function requestJsonData(url, success, error, complete) {
    return $.ajax({
        dataType: "json",
        type: 'GET',
        url: url,
        data: null,
        success: success,
        error: error,
        complete: complete
    });
}
