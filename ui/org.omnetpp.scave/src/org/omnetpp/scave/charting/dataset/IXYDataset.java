/*--------------------------------------------------------------*
  Copyright (C) 2006-2015 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  'License' for details on this and other legal matters.
*--------------------------------------------------------------*/

package org.omnetpp.scave.charting.dataset;

import org.omnetpp.common.engine.BigDecimal;

/**
 * Interface for content of charts displaying x-y series.
 * Within each series the items are ordered according to
 * x coordinates.
 *
 * @author tomi
 */
public interface IXYDataset extends IDataset {

    public enum Type {
        Int,
        Double,
        Enum
    }

    public enum InterpolationMode {
        Unspecified,
        None,
        Linear,
        SampleHold,
        BackwardSampleHold
    }

    /**
     * Returns the number of series in the dataset.
     *
     * @return The series count.
     */
    public int getSeriesCount();

    /**
     * Returns the key for a series.
     *
     * @param series  the series (zero-based index).
     * @return The key for the series.
     */
    public String getSeriesKey(int series);

    /**
     * Returns the display name of the series.
     */
    public String getSeriesTitle(int series);

    /**
     * Returns the type of the y values in the specified series.
     */
    public Type getSeriesType(int series);

    /**
     * Returns the interpolation mode of the specified series.
     */
    public InterpolationMode getSeriesInterpolationMode(int series);

    /**
     * Returns the number of items in a series.
     *
     * @param series  the series index (zero-based).
     * @return The item count.
     */
    public int getItemCount(int series);

    /**
     * Returns the x-value for an item within a series.
     * X values are assumed to be in ascending order.
     * NaNs are not allowed.
     *
     * @param series  the series index (zero-based).
     * @param item  the item index (zero-based).
     * @return The x-value.
     */
    public double getX(int series, int item);

    /**
     * For tooltips.
     */
    public String getXAsString(int series, int item);

    /**
     * Returns the x-value as a BigDecimal for an item within a series.
     *
     * @param series  the series index (zero-based).
     * @param item  the item index (zero-based).
     * @return The x-value.
     */
    public BigDecimal getPreciseX(int series, int item);

    /**
     * Returns the minimum value of the x coordinates.
     */
    public double getMinX();

    /**
     * Returns the maximum value of the x coordinates.
     */
    public double getMaxX();

    /**
     * Returns the y-value for an item within a series.
     * It may return NaN if the corresponding x does not
     * have a y.
     *
     * @param series  the series index (zero-based).
     * @param item  the item index (zero-based).
     * @return The y-value.
     */
    public double getY(int series, int item);

    /**
     * For tooltips.
     */
    public String getYAsString(int series, int item);

    /**
     * Returns the y-value as a BigDecimal.
     *
     * @param series  the series index (zero-based).
     * @param item  the item index (zero-based).
     * @return The y-value.
     */
    public BigDecimal getPreciseY(int series, int item);

    /**
     * Returns the minimum value of the y coordinates.
     */
    public double getMinY();

    /**
     * Returns the maximum value of the y coordinates.
     */
    public double getMaxY();
}
