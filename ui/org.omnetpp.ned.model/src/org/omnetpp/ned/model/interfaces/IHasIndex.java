package org.omnetpp.ned.model.interfaces;

/**
 * Objects that can have an optional index (ie. they can be vector or non vector)
 * @author rhornig
 */
public interface IHasIndex extends IHasName {

    public String getVectorSize();

    public void setVectorSize(String indexstring);

    public String getNameWithIndex();

}
