/*--------------------------------------------------------------*
  Copyright (C) 2006-2008 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  'License' for details on this and other legal matters.
*--------------------------------------------------------------*/

package org.omnetpp.ned.editor.text.actions;

import org.eclipse.core.resources.IFile;
import org.eclipse.ui.part.FileEditorInput;
import org.eclipse.ui.texteditor.ITextEditor;
import org.omnetpp.ned.core.NEDResourcesPlugin;
import org.omnetpp.ned.core.refactoring.RefactoringTools;
import org.omnetpp.ned.editor.text.TextualNedEditor;
import org.omnetpp.ned.model.ex.NedFileElementEx;

/**
 * Organize "import" lines in a NED file.
 *
 * @author andras
 */
public class OrganizeImportsAction extends NedTextEditorAction {
    public static final String ID = "OrganizeImports";

    public OrganizeImportsAction(ITextEditor editor) {
        super(ID, editor);
    }

    @Override
	public void update() {
    	setEnabled(getTextEditor() != null && !getNedFileElement().hasSyntaxError());
	}

	@Override
	protected void doRun() {
        NedFileElementEx nedFileElement = getNedFileElement();
        if (nedFileElement.hasSyntaxError())
        	return; // don't mess with the source while it has syntax error

        // fix the package as a bonus
        RefactoringTools.fixupPackageDeclaration(nedFileElement);

        // organize imports
        RefactoringTools.organizeImports(nedFileElement);

        // update text editor
        ((TextualNedEditor)getTextEditor()).pullChangesFromNEDResources();
        //((TextualNedEditor)getTextEditor()).setText(nedFileElement.getNEDSource());
    }

	protected NedFileElementEx getNedFileElement() {
	    IFile file = ((FileEditorInput)getTextEditor().getEditorInput()).getFile();
        return NEDResourcesPlugin.getNEDResources().getNedFileElement(file);
	}
}
