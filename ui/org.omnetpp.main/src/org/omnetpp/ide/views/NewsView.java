package org.omnetpp.ide.views;


import org.eclipse.swt.SWT;
import org.eclipse.swt.browser.Browser;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.ui.part.ViewPart;
import org.omnetpp.common.IConstants;


/**
 * This view displays a news page from the main omnetpp webpage or
 * can be used to display any URL in a view especially the version list 
 * page that is displayed if a new version is found.
 */

public class NewsView extends ViewPart {
	protected String urlToShow = "";
	protected Browser browser; 

	/**
	 * This is a callback that will allow us
	 * to create the viewer and initialize it.
	 */
	public void createPartControl(Composite parent) {
		browser = new Browser(parent,SWT.NONE);
		browser.setUrl(urlToShow);
	}
	/**
	 * Passing the focus request to the viewer's control.
	 */
	public void setFocus() {
		browser.setFocus();
	}
	
	public void setURL(String url) {
		urlToShow = url;
		if (browser != null)
			browser.setUrl(url);
	}
	
}