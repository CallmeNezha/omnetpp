package org.omnetpp.cdt.wizard;

import org.eclipse.cdt.managedbuilder.ui.wizards.MBSCustomPage;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;

public class MakeMakeConfigPage extends MBSCustomPage {

    private Composite composite;

	public MakeMakeConfigPage() {
    	this("org.omnetpp.cdt.wizardPage1");
        // TODO Auto-generated constructor stub
    }

    public MakeMakeConfigPage(String pageID) {
        super(pageID);
        // TODO Auto-generated constructor stub
    }

    @Override
    protected boolean isCustomPageComplete() {
        // TODO Auto-generated method stub
        return true;
    }

    public String getName() {
        // TODO Auto-generated method stub
        return "test";
    }

    public void createControl(Composite parent) {
    	composite = new Composite(parent, SWT.NONE);
    }

    public void dispose() {
        // TODO Auto-generated method stub

    }

    public Control getControl() {
        return composite;
    }

    public String getDescription() {
        // TODO Auto-generated method stub
        return "";
    }

    public String getErrorMessage() {
        return null;
    }

    public Image getImage() {
        // TODO Auto-generated method stub
        return null;
    }

    public String getMessage() {
        // TODO Auto-generated method stub
        return "msg";
    }

    public String getTitle() {
        // TODO Auto-generated method stub
        return "Example page title";
    }

    public void performHelp() {
        // TODO Auto-generated method stub

    }

    public void setDescription(String description) {
        // TODO Auto-generated method stub

    }

    public void setImageDescriptor(ImageDescriptor image) {
        // TODO Auto-generated method stub

    }

    public void setTitle(String title) {
        // TODO Auto-generated method stub

    }

    public void setVisible(boolean visible) {
        // TODO Auto-generated method stub

    }
    
    

}
