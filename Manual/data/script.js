/* Creates the table of contents based on the h2 and beyond tags.
 * Then, it places it after the opening paragraphs.
 */
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
      var new_ul = document.createElement('ul');
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


/* Creates a header, including breadcrumbs for the current page.
 * title:
 *   Current page title.
 * bc_names:
 *   List of names of the pages in the breadcrumbs. The home page must not be
 *   included. Undefined for none.
 * bc_links:
 *   Same as bc_names, but for the links.
 */
function create_header(title, bc_names, bc_links) {
  var header_div = document.createElement('div');
  header_div.innerHTML =
    '<img src="../images/logo.png" style="width: 24px; height: 24px; margin-right: 8px;">' +
    '<b><i>Pikifen manual</i></b> ' +
    '<span style="margin-left: 10px; margin-right: 10px;">|</span> ' +
    '<a href="#top" title="Go to the top of the page.">&UpArrowBar;</a> ' +
    '<span style="margin-left: 10px; margin-right: 10px;">|</span> ' +
    '<span id="breadcrumbs"></span>';
  header_div.id = 'header';
  content_div = document.getElementById('content');
  content_div.parentNode.insertBefore(header_div, content_div);

  var bc_span = document.getElementById('breadcrumbs');
  var br_result = '';

  if(title != 'Home') {
    br_result += '<a href="home.html" target="_parent">Home</a> &gt; ';
  }
  if(bc_names !== undefined || bc_links !== undefined) {
    for(var b = 0; b < bc_names.length; b++) {
      br_result += '<a href="' + bc_links[b] + '" target="_parent">' + bc_names[b] + '</a> &gt; ';
    }
  }
  br_result += title;
  bc_span.innerHTML = br_result;
}


/* Adjusts the page's title and first h1 header.
 * title:
 *   Current page title.
 */
function set_title(title) {
  document.title = title + ' - Pikifen manual';
  var title_h1 = document.createElement('h1');
  title_h1.innerHTML = title;
  var content_div = document.getElementById('content');
  content_div.insertBefore(title_h1, content_div.firstChild);
}


/* Sets up the page.
 * title:
 *   Current page title.
 * bc_names:
 *   List of names of the pages in the breadcrumbs. The home page must not be
 *   included. Undefined for none.
 * bc_links:
 *   Same as bc_names, but for the links.
 * use_toc:
 *   True to use table of contents.
 */
function setup(title, bc_names, bc_links, use_toc) {
  if(use_toc === undefined) use_toc = true;

  set_title(title);
  create_header(title, bc_names, bc_links);
  if(use_toc) make_toc();
}


/* Makes scrolling to a section end up slightly before the section's header,
 * so that the page header can be taken into account.
 * Solution by Ian Clark from https://stackoverflow.com/a/13067009
 */
(function(document, history, location) {
  var HISTORY_SUPPORT = !!(history && history.pushState);

  var anchorScrolls = {
    ANCHOR_REGEX: /^#[^ ]+$/,
    OFFSET_HEIGHT_PX: 30,

    /**
     * Establish events, and fix initial scroll position if a hash is provided.
     */
    init: function() {
      this.scrollToCurrent();
      window.addEventListener('hashchange', this.scrollToCurrent.bind(this));
      document.body.addEventListener('click', this.delegateAnchors.bind(this));
    },

    /**
     * Return the offset amount to deduct from the normal scroll position.
     * Modify as appropriate to allow for dynamic calculations
     */
    getFixedOffset: function() {
      return this.OFFSET_HEIGHT_PX;
    },

    /**
     * If the provided href is an anchor which resolves to an element on the
     * page, scroll to it.
     * @param  {String} href
     * @return {Boolean} - Was the href an anchor.
     */
    scrollIfAnchor: function(href, pushToHistory) {
      var match, rect, anchorOffset;

      if(!this.ANCHOR_REGEX.test(href)) {
        return false;
      }

      match = document.getElementById(href.slice(1));

      if(match) {
        rect = match.getBoundingClientRect();
        anchorOffset = window.pageYOffset + rect.top - this.getFixedOffset();
        window.scrollTo(window.pageXOffset, anchorOffset);

        // Add the state to history as-per normal anchor links
        if(HISTORY_SUPPORT && pushToHistory) {
          history.pushState({}, document.title, location.pathname + href);
        }
      }

      return !!match;
    },

    /**
     * Attempt to scroll to the current location's hash.
     */
    scrollToCurrent: function() {
      this.scrollIfAnchor(window.location.hash);
    },

    /**
     * If the click event's target was an anchor, fix the scroll position.
     */
    delegateAnchors: function(e) {
      var elem = e.target;

      if(
        elem.nodeName === 'A' &&
        this.scrollIfAnchor(elem.getAttribute('href'), true)
      ) {
        e.preventDefault();
      }
    }
  };

  window.addEventListener(
    'DOMContentLoaded', anchorScrolls.init.bind(anchorScrolls)
  );
})(window.document, window.history, window.location);
