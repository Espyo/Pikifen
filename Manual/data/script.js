function make_toc () {
  var headings = [].slice.call(document.body.querySelectorAll('h2, h3, h4, h5, h6'));
  
  if(headings.length == 0) {
    return;
  }
  
  var toc = document.createElement('div');
  toc.id = 'toc';
  
  var toc_title = document.createElement('a');
  toc_title.textContent = "Index";
  toc.appendChild(toc_title);
  
  headings[0].parentNode.insertBefore(toc, headings[0]);
  
  var prev_level = -1;
  var cur_ul = document.createElement('ul');
  toc.appendChild(cur_ul);
  
  for(var h = 0; h < headings.length; h++) {
    var level = headings[h].tagName[1];
    if(prev_level == -1) {
      prev_level = level;
    }
    
    if(level > prev_level) {
      new_ul = document.createElement('ul');
      cur_ul.appendChild(new_ul);
      cur_ul = new_ul;
    } else if(level < prev_level) {
      cur_ul = cur_ul.parentNode;
    }

    var link = document.createElement('a');
    if(headings[h].id != '') {
      link.setAttribute('href', '#' + headings[h].id);
      link.textContent = headings[h].textContent;
    } else {
      link.textContent = '!!!!!!!!!!!!!ERROR: H WITH NO ID: ' + headings[h].textContent + '!!!!!!!!!!!!!';
    }
    
    var li = document.createElement('li');
    li.appendChild(link);
    
    cur_ul.appendChild(li);
    
    prev_level = level;
  }
}
