

//success(data)
//error(xhr, status, error)
function requestJsonData(url, success, error, complete, timeout, async) {
    return $.ajax({
        async: async == null ? true : async,
        dataType: "json",
        type: 'GET',
        url: url,
        data: null,
        success: success,
        error: error,
        complete: complete,
        timeout: timeout == null ? 3000 : timeout
    });
}
