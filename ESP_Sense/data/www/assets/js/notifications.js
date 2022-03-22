var notificationBox;

function createNotificationBox(){
    let box = new ElementBuilder('aside', 'notifications').Create();
    
    let tab = new ElementBuilder('div', null, 'tab').Create();
    let tabIcon = new ElementBuilder('i', null, 'fa fa-envelope-o').Create();
    
    let expandable = new ElementBuilder('div', null, 'expandable').Create();
    
    let head = new ElementBuilder('div', null, 'header').Create();
    let headSpan = new ElementBuilder('span', null, null, null, 'Notifications').Create();
    let headBut = new ElementBuilder('button', null, 'btn', {key:'type', value:'button'}, 'Clear').Create();
    let noteList = new ElementBuilder('div', null, 'note-list').Create();
    
    tab.append(tabIcon);
    
    head.append(headSpan);
    head.append(headBut);
    
    expandable.append(head);
    expandable.append(noteList);  
    box.append(tab);
    box.append(expandable);
    $('body').append(box);  
    
    headBut.click(clearNotifications);
    
    tab.click(showNotifications);
    
    expandable.mouseleave(hideNotifications);
    
    return box;
}

function addNotification(iconClass, message, clickfn){
    let note = new ElementBuilder('notification', null, 'notification').Create();
    
    let iconCon = new ElementBuilder('div', null, 'note-icon-con').Create();
    let icon = new ElementBuilder('i', null, iconClass).Create();
    iconCon.append(icon);
    
    let msgCon = new ElementBuilder('div', null, 'note-msg-con').Create();
    let msg = new ElementBuilder('span', null, null, null, message).Create();
    msgCon.append(msg);
    
    let detailCon = new ElementBuilder('div', null, 'note-details-con').Create();
    let removeBut = new ElementBuilder('i', null, 'fa fa-remove').Create();
    detailCon.append(removeBut);
    
    note.append(iconCon);
    note.append(msgCon);
    note.append(detailCon);
    
    let list = notificationBox.find('.note-list');
    list.append(note);
    notificationBox.attr('value', list.children().length.toString());
    
    note.on('click', function(e){
        if($(e.target).is('i.fa.fa-remove')) return;
        clickfn();
    });
    removeBut.on('click', function(){
        removeNotification(note);
        setTimeout(function(){
            note.remove();
            notificationBox.attr('value', list.children().length.toString());
        }, 500);
    });
    
    return note;
}

function removeNotification(elem){
    elem = $(elem);
    
    elem.addClass('remove');
}

function clearNotifications(){    
    hideNotifications();
    setTimeout(function(){
        notificationBox.find('.note-list').empty();
        notificationBox.attr('value', '0');
    }, 1000);    
}

function showNotifications(){
    if(notificationBox.find('.note-list').children().length == 0) return;
    notificationBox.addClass('view');
}

function hideNotifications(){
    notificationBox.removeClass('view');
}

EventReady.add(function(){
    notificationBox = $('aside#notifications');
    if(notificationBox.length == 0){
        notificationBox = createNotificationBox();
    }
    
    if(devMode){
        addNotification('fa fa-code', 'Update Available', function(){window.location.href = '/update.html';});
        addNotification('fa fa-code', 'Firmware and Website Version do not match', function(){openInNewTab('https://github.com/AddioElectronics/ESP-Sense');});
    }
    
});