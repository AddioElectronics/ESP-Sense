

//success(data)
//error(xhr, status, error)
function requestJsonData(url, success, error, complete, timeout) {
    return $.ajax({
        dataType: "json",
        type: 'GET',
        url: url,
        data: null,
        success: success,
        error: error,
        complete: complete,
        timeout: timeout == undefined ? 3000 : timeout
    });
}
