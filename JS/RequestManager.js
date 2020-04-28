function get(url){
    var request = new XMLHttpRequest();
    var response = "";
    request.open('GET', url);
    request.onreadystatechange = function() {
        if (request.readyState != 4) {

        } else if (request.status != 200) {

        } else {
            response = request.responseText;
        }
    }
    request.send(null);
}

function post(url, data) {
    var request = new XMLHttpRequest();
    var response = "";
    request.open('POST', url);
    request.onreadystatechange = function () {
        if (request.readyState != 4) {
            // リクエスト中
        } else if (request.status != 200) {
            // 失敗
        } else {
            // 送信成功
            // var result = request.responseText;
        }
    };
    request.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
    request.send(data);
}
