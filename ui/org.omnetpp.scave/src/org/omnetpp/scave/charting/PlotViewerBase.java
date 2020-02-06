/*--------------------------------------------------------------*
  Copyright (C) 2006-2015 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  'License' for details on this and other legal matters.
*--------------------------------------------------------------*/

package org.omnetpp.scave.charting;

import static org.omnetpp.scave.charting.properties.PlotDefaults.DEFAULT_INSETS_BACKGROUND_COLOR;
import static org.omnetpp.scave.charting.properties.PlotDefaults.DEFAULT_INSETS_LINE_COLOR;
import static org.omnetpp.scave.charting.properties.PlotProperties.PROP_ANTIALIAS;
import static org.omnetpp.scave.charting.properties.PlotProperties.PROP_BACKGROUND_COLOR;
import static org.omnetpp.scave.charting.properties.PlotProperties.PROP_CACHING;
import static org.omnetpp.scave.charting.properties.PlotProperties.PROP_DISPLAY_LEGEND;
import static org.omnetpp.scave.charting.properties.PlotProperties.PROP_LEGEND_ANCHORING;
import static org.omnetpp.scave.charting.properties.PlotProperties.PROP_LEGEND_BORDER;
import static org.omnetpp.scave.charting.properties.PlotProperties.PROP_LEGEND_FONT;
import static org.omnetpp.scave.charting.properties.PlotProperties.PROP_LEGEND_POSITION;
import static org.omnetpp.scave.charting.properties.PlotProperties.PROP_PLOT_TITLE;
import static org.omnetpp.scave.charting.properties.PlotProperties.PROP_PLOT_TITLE_FONT;
import static org.omnetpp.scave.charting.properties.PlotProperties.PROP_Y_AXIS_MAX;
import static org.omnetpp.scave.charting.properties.PlotProperties.PROP_Y_AXIS_MIN;

import org.apache.commons.lang3.ObjectUtils;
import org.eclipse.core.runtime.Assert;
import org.eclipse.core.runtime.ListenerList;
import org.eclipse.draw2d.Graphics;
import org.eclipse.draw2d.SWTGraphics;
import org.eclipse.draw2d.geometry.Insets;
import org.eclipse.draw2d.geometry.Rectangle;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ControlAdapter;
import org.eclipse.swt.events.ControlEvent;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.RGB;
import org.eclipse.swt.widgets.Composite;
import org.omnetpp.common.Debug;
import org.omnetpp.common.canvas.ICoordsMapping;
import org.omnetpp.common.canvas.RectangularArea;
import org.omnetpp.common.canvas.ZoomableCachingCanvas;
import org.omnetpp.common.canvas.ZoomableCanvasMouseSupport;
import org.omnetpp.common.color.ColorFactory;
import org.omnetpp.common.util.Converter;
import org.omnetpp.common.util.GraphicsUtils;
import org.omnetpp.common.util.StringUtils;
import org.omnetpp.scave.ScavePlugin;
import org.omnetpp.scave.charting.dataset.IDataset;
import org.omnetpp.scave.charting.properties.PlotDefaults;
import org.omnetpp.scave.charting.properties.PlotProperties.LegendAnchor;
import org.omnetpp.scave.charting.properties.PlotProperties.LegendPosition;

/**
 * Base class for all plot widgets.
 *
 * @author tomi, andras
 */
public abstract class PlotViewerBase extends ZoomableCachingCanvas implements IPlotViewer {
    private static final boolean debug = false;

    protected static final String[] PLOTBASE_PROPERTY_NAMES = new String[] {
            PROP_PLOT_TITLE,
            PROP_PLOT_TITLE_FONT,
            PROP_DISPLAY_LEGEND,
            PROP_LEGEND_BORDER,
            PROP_LEGEND_FONT,
            PROP_LEGEND_POSITION,
            PROP_LEGEND_ANCHORING,
            PROP_ANTIALIAS,
            PROP_CACHING,
            PROP_BACKGROUND_COLOR,
            PROP_Y_AXIS_MIN,
            PROP_Y_AXIS_MAX
    };

    // when displaying confidence intervals, XXX chart parameter?
    protected static final double CONFIDENCE_LEVEL = 0.95;

    protected boolean antialias;
    protected Color backgroundColor;
    protected Title title = new Title();
    protected String titleText;
    protected Legend legend = new Legend();
    protected LegendTooltip legendTooltip;

    private String statusText = "No data available."; // displayed when there's no dataset
    private String warningText = null;

    /* bounds specified by the user*/
    protected RectangularArea userDefinedArea =
        new RectangularArea(Double.NEGATIVE_INFINITY, Double.NEGATIVE_INFINITY,
                Double.POSITIVE_INFINITY, Double.POSITIVE_INFINITY);
    /* bounds calculated from the dataset */
    protected RectangularArea chartArea;
    /* area to be displayed */
    private RectangularArea zoomedArea;

    private ZoomableCanvasMouseSupport mouseSupport;
    private Color insetsBackgroundColor = DEFAULT_INSETS_BACKGROUND_COLOR; //TODO make this a proper property
    private Color insetsLineColor = DEFAULT_INSETS_LINE_COLOR; //TODO make this a proper property

    protected IPlotSelection selection;
    private ListenerList<IChartSelectionListener> listeners = new ListenerList<>();

    private int layoutDepth = 0; // how many layoutChart() calls are on the stack
    private IDataset dataset;

    public PlotViewerBase(Composite parent, int style) {
        super(parent, style);
        setToolTipText(null); // prevents "Close" tooltip of the TabItem from coming up (Linux only)

        legendTooltip = new LegendTooltip(this);

        mouseSupport = new ZoomableCanvasMouseSupport(this); // add mouse handling; may be made optional

        addControlListener(new ControlAdapter() {
            @Override
            public void controlResized(ControlEvent e) {
                layoutChart();
            }
        });
    }

    /**
     * Sets the data to be visualized by the chart.
     */
    public void setDataset(IDataset dataset) {
        if (debug) Debug.println("setDataset()");
        doSetDataset(dataset);
        this.dataset = dataset;
        updateTitle();
        updateZoomedArea();
    }

    abstract void doSetDataset(IDataset dataset);

    abstract String getHoverHtmlText(int x, int y);

    protected void layoutChart() {
        // prevent nasty infinite layout recursions
        if (layoutDepth>0)
            return;

        // ignore initial invalid layout request
        if (getClientArea().isEmpty())
            return;

        layoutDepth++;
        if (debug) Debug.println("layoutChart(), level "+layoutDepth);
        Image image = new Image(getDisplay(), 1, 1);
        GC gc = new GC(image);
        Graphics graphics = new SWTGraphics(gc);
        try {
            // preserve zoomed-out state while resizing
            boolean shouldZoomOutX = getZoomX()==0 || isZoomedOutX();
            boolean shouldZoomOutY = getZoomY()==0 || isZoomedOutY();

            for (int pass = 1; pass <= 2; ++pass) {
                Rectangle plotArea = doLayoutChart(graphics, pass);
                setViewportRectangle(plotArea);

                if (shouldZoomOutX)
                    zoomToFitX();
                if (shouldZoomOutY)
                    zoomToFitY();
                validateZoom(); //Note: scrollbar.setVisible() triggers Resize too
            }
        }
        catch (Throwable e) {
            ScavePlugin.logError(e);
        }
        finally {
            layoutDepth--;
            graphics.dispose();
            gc.dispose();
            image.dispose();
        }
        // may trigger another layoutChart()
        updateZoomedArea();
    }

    /**
     * Sets the zoomed area of the chart.
     * The change will be applied when the chart is layouted next time.
     */
    public void setZoomedArea(RectangularArea area) {
        if (dataset != null && !getClientArea().isEmpty())
            zoomToArea(area);
        else
            this.zoomedArea = area;
    }

    /**
     * Applies the zoomed area was set by {@code setZoomedArea}.
     * To be called after the has an area (calculated from the dataset)
     * and the virtual size of the canvas is calculated.
     */
    private void updateZoomedArea() {
        if (zoomedArea != null && !getBounds().isEmpty() && dataset != null) {
            if (debug) Debug.format("Restoring zoomed area: %s%n", zoomedArea);
            RectangularArea area = zoomedArea;
            zoomedArea = null;
            zoomToArea(area);
        }
    }

    /**
     * Calculate positions of chart elements such as title, legend, axis labels, plot area.
     * @param pass TODO
     */
    abstract protected Rectangle doLayoutChart(Graphics graphics, int pass);

    /*-------------------------------------------------------------------------------------------
     *                                      Drawing
     *-------------------------------------------------------------------------------------------*/
    private ICoordsMapping coordsMapping;

    @Override
    protected void paintCachableLayer(Graphics graphics) {
        if (debug) {
            Debug.println("paintCachableLayer()");
            Debug.println(String.format("area=%f, %f, %f, %f, zoom: %f, %f", getMinX(), getMaxX(), getMinY(), getMaxY(), getZoomX(), getZoomY()));
            Debug.println(String.format("view port=%s, vxy=%d, %d", getViewportRectangle(), getViewportLeft(), getViewportTop()));
        }
        if (getClientArea().isEmpty())
            return;

        coordsMapping = getOptimizedCoordinateMapper();
        resetDrawingStylesAndColors(graphics);
        doPaintCachableLayer(graphics, coordsMapping);
    }

    @Override
    protected void paintNoncachableLayer(Graphics graphics) {
        if (debug) Debug.println("paintNoncachableLayer()");
        if (getClientArea().isEmpty())
            return;

        if (coordsMapping == null)
            coordsMapping = getOptimizedCoordinateMapper();
        resetDrawingStylesAndColors(graphics);

        doPaintNoncachableLayer(graphics, coordsMapping);

        coordsMapping = null;
    }

    abstract protected void doPaintCachableLayer(Graphics graphics, ICoordsMapping coordsMapping);
    abstract protected void doPaintNoncachableLayer(Graphics graphics, ICoordsMapping coordsMapping);

    /**
     *
     * @return
     */
    protected abstract RectangularArea calculatePlotArea();

    public IPlotSelection getSelection() {
        return selection;
    }

    public void setSelection(IPlotSelection selection) {
        if (!ObjectUtils.equals(this.selection, selection)) {
            this.selection = selection;
            chartChanged();
            fireChartSelectionChange(selection);
        }
    }

    public void addChartSelectionListener(IChartSelectionListener listener) {
        listeners.add(listener);
    }

    public void removeChartSelectionListener(IChartSelectionListener listener) {
        listeners.remove(listener);
    }

    protected void fireChartSelectionChange(IPlotSelection selection) {
        for (Object listener : listeners.getListeners())
            ((IChartSelectionListener)listener).selectionChanged(selection);
    }

    /**
     * Switches between zoom and pan mode.
     * @param mouseMode should be ZoomableCanvasMouseSupport.PAN_MODE or ZoomableCanvasMouseSupport.ZOOM_MODE
     */
    public void setMouseMode(int mouseMode) {
        mouseSupport.setMouseMode(mouseMode);
    }

    public int getMouseMode() {
        return mouseSupport.getMouseMode();
    }

    public String getStatusText() {
        return statusText;
    }

    public void setStatusText(String statusText) {
        this.statusText = statusText;
        chartChanged();
    }

    public String getWarningText() {
        return warningText;
    }

    public void setWarningText(String warningText) {
        this.warningText = warningText;
        chartChanged();
    }

    /*----------------------------------------------------------------------
     *                            Properties
     *----------------------------------------------------------------------*/

    public String[] getPropertyNames() {
        return PLOTBASE_PROPERTY_NAMES;
    }

    public void setProperty(String name, String value) {
        // Titles
        if (PROP_PLOT_TITLE.equals(name))
            setTitle(value);
        else if (PROP_PLOT_TITLE_FONT.equals(name))
            setTitleFont(Converter.tolerantStringToOptionalSwtfont(value));
        // Legend
        else if (PROP_DISPLAY_LEGEND.equals(name))
            setDisplayLegend(Converter.tolerantStringToOptionalBoolean(value));
        else if (PROP_LEGEND_BORDER.equals(name))
            setLegendBorder(Converter.tolerantStringToOptionalBoolean(value));
        else if (PROP_LEGEND_FONT.equals(name))
            setLegendFont(Converter.tolerantStringToOptionalSwtfont(value));
        else if (PROP_LEGEND_POSITION.equals(name))
            setLegendPosition(Converter.tolerantStringToOptionalEnum(value, LegendPosition.class));
        else if (PROP_LEGEND_ANCHORING.equals(name))
            setLegendAnchor(Converter.tolerantStringToOptionalEnum(value, LegendAnchor.class));
        // Plot
        else if (PROP_ANTIALIAS.equals(name))
            setAntialias(Converter.tolerantStringToOptionalBoolean(value));
        else if (PROP_CACHING.equals(name))
            setCaching(Converter.tolerantStringToOptionalBoolean(value));
        else if (PROP_BACKGROUND_COLOR.equals(name))
            setBackgroundColor(ColorFactory.asRGB(value));
        // Axes
        else if (PROP_Y_AXIS_MIN.equals(name))
            setYMin(Converter.tolerantStringToOptionalDouble(value));
        else if (PROP_Y_AXIS_MAX.equals(name))
            setYMax(Converter.tolerantStringToOptionalDouble(value));
        else
            System.out.println("Unrecognized chart property: " + name + " = " + value);
            //ScavePlugin.logError(new RuntimeException("unrecognized chart property: "+name));
    }

    protected void resetProperties() {
        for (String name : getPropertyNames()) {
            String value = PlotDefaults.getDefaultPropertyValueAsString(name);
            if (value == null)
                Debug.println("WARNING: Property " + name + " has no default value");
            else
                setProperty(name, value);
        }
    }

    /*
     * Clears all data and resets the visual properties to their defaults.
     */
    public void clear() {
        resetProperties();
    }

    protected String getElementId(String propertyName) {
        int index = propertyName.indexOf('/');
        return index >= 0 ? propertyName.substring(index + 1) : null;
    }

    public boolean getAntialias() {
        return antialias;
    }

    public void setAntialias(boolean antialias) {
        this.antialias = antialias;
        chartChanged();
    }

    public void setCaching(boolean caching) {
        super.setCaching(caching);
        chartChanged();
    }

    public void setBackgroundColor(RGB rgb) {
        Assert.isNotNull(rgb);
        this.backgroundColor = new Color(null, rgb);
        chartChanged();
    }

    public void setTitle(String value) {
        Assert.isNotNull(value);
        titleText = value;
        updateTitle();
    }

    private void updateTitle() {
        String newTitle = titleText;
        if (dataset != null)
            newTitle = StringUtils.defaultString(dataset.getTitle(titleText), newTitle);
        if (!ObjectUtils.equals(newTitle, title.getText())) {
            title.setText(newTitle);
            chartChanged();
        }
    }

    public void setTitleFont(Font value) {
        Assert.isNotNull(value);
        title.setFont(value);
        chartChanged();
    }

    public abstract void setDisplayAxesDetails(boolean value);

    public void setDisplayTitle(boolean value) {
        title.setVisible(value);
        chartChanged();
    }

    public void setDisplayLegend(boolean value) {
        legend.setVisible(value);
        chartChanged();
    }

    public void setDisplayLegendTooltip(boolean value) {
        legendTooltip.setVisible(value);
        chartChanged();
    }

    public void setLegendBorder(boolean value) {
        legend.setDrawBorder(value);
        chartChanged();
    }

    public void setLegendFont(Font value) {
        Assert.isNotNull(value);
        legend.setFont(value);
        chartChanged();
    }

    public void setLegendPosition(LegendPosition value) {
        Assert.isNotNull(value);
        legend.setPosition(value);
        chartChanged();
    }

    public void setLegendAnchor(LegendAnchor value) {
        Assert.isNotNull(value);
        legend.setAnchor(value);
        chartChanged();
    }

    public void setXMin(double value) {
        userDefinedArea.minX = value;
        updateArea();
        chartChanged();
    }

    public void setXMax(double value) {
        userDefinedArea.maxX = value;
        updateArea();
        chartChanged();
    }

    public void setYMin(double value) {
        userDefinedArea.minY = value;
        updateArea();
        chartChanged();
    }

    public void setYMax(double value) {
        userDefinedArea.maxY = value;
        updateArea();
        chartChanged();
    }

    /**
     * Sets the area of the zoomable canvas.
     * This method is called when the area changes because
     * <ul>
     * <li> the dataset changed and a new chart area calculated
     * <li> the user changed the bounds of the are by setting a chart property
     * </ul>
     */
    protected void updateArea() {
        if (chartArea == null)
            return;

        RectangularArea area = transformArea(userDefinedArea);
        area.minX = Double.isInfinite(area.minX) ? chartArea.minX : area.minX;
        area.minY = Double.isInfinite(area.minY) ? chartArea.minY : area.minY;
        area.maxX = Double.isInfinite(area.maxX) ? chartArea.maxX : area.maxX;
        area.maxY = Double.isInfinite(area.maxY) ? chartArea.maxY : area.maxY;

        if (!area.equals(getArea())) {
            if (debug) Debug.format("Update area: %s --> %s%n", getArea(), area);
            setArea(area);
        }
    }

    protected RectangularArea transformArea(RectangularArea area) {
        double minX = transformX(area.minX);
        double minY = transformY(area.minY);
        double maxX = transformX(area.maxX);
        double maxY = transformY(area.maxY);

        return new RectangularArea(
            Double.isNaN(minX) || Double.isInfinite(minX) ? Double.NEGATIVE_INFINITY : minX,
            Double.isNaN(minY) || Double.isInfinite(minY)? Double.NEGATIVE_INFINITY : minY,
            Double.isNaN(maxX) || Double.isInfinite(maxX)? Double.POSITIVE_INFINITY : maxX,
            Double.isNaN(maxY) || Double.isInfinite(maxY) ? Double.POSITIVE_INFINITY : maxY
        );
    }

    protected double transformX(double x) {
        return x;
    }

    protected double transformY(double y) {
        return y;
    }

    protected double inverseTransformX(double x) {
        return x;
    }

    protected double inverseTransformY(double y) {
        return y;
    }


    // make public
    @Override
    public ICoordsMapping getOptimizedCoordinateMapper() {
        return super.getOptimizedCoordinateMapper();
    }

    /**
     * Resets all graphics settings except clipping and transform.
     */
    public void resetDrawingStylesAndColors(Graphics graphics) {
        graphics.setAntialias(antialias ? SWT.ON : SWT.OFF);
        graphics.setAlpha(255);
        graphics.setBackgroundColor(backgroundColor);
        graphics.setForegroundColor(PlotDefaults.DEFAULT_FOREGROUND_COLOR);
        graphics.setLineWidth(0);
        graphics.setLineStyle(SWT.LINE_SOLID);
        //graphics.setFillRule();
        //graphics.setLineCap();
        //graphics.setLineJoin();
        //graphics.setXORMode(false);
        //graphics.setFont(null);
        // TODO: these operations are not supported by SVGGraphics yet
        if (!GraphicsUtils.isSVGGraphics(graphics)) {
            graphics.setBackgroundPattern(null);
            graphics.setForegroundPattern(null);
            graphics.setInterpolation(SWT.DEFAULT);
            graphics.setLineDash((int[])null);
            graphics.setTextAntialias(SWT.DEFAULT);
        }
    }

    protected void chartChanged() {
        layoutChart();
        clearCanvasCacheAndRedraw();
    }

    protected void paintInsets(Graphics graphics) {
        Insets insets = getInsets();
        // do not draw insets border when exporting to SVG
        if (!GraphicsUtils.isSVGGraphics(graphics)) {
            Rectangle canvasRect = new Rectangle(getClientArea());
            graphics.setForegroundColor(insetsBackgroundColor);
            graphics.setBackgroundColor(insetsBackgroundColor);
            graphics.fillRectangle(0, 0, canvasRect.width, insets.top); // top
            graphics.fillRectangle(0, canvasRect.bottom()-insets.bottom, canvasRect.width, insets.bottom); // bottom
            graphics.fillRectangle(0, 0, insets.left, canvasRect.height); // left
            graphics.fillRectangle(canvasRect.right()-insets.right, 0, insets.right, canvasRect.height); // right
            graphics.setForegroundColor(insetsLineColor);
        }
        graphics.drawRectangle(insets.left, insets.top, getViewportWidth(), getViewportHeight());
    }

    protected void drawStatusText(Graphics graphics) {
        Rectangle rect = getViewportRectangle();
        int warningY = rect.y+10;

        if (getStatusText() != null && !StringUtils.isBlank(getStatusText())) {
            resetDrawingStylesAndColors(graphics);
            graphics.drawText(getStatusText(), rect.x+10, rect.y+10);
            warningY += 20;
        }

        if (getWarningText() != null) {
            resetDrawingStylesAndColors(graphics);
            graphics.setForegroundColor(new Color(getDisplay(), 255, 0, 0));
            graphics.drawText(getWarningText(), rect.x+10, warningY);
        }
    }

    protected void drawRubberband(Graphics graphics) {
        mouseSupport.drawRubberband(graphics);
    }

    protected String formatValue(double value, double halfInterval) {
        return !Double.isNaN(halfInterval) && halfInterval > 0.0 ?
                String.format("%.3g\u00b1%.3g", value, halfInterval) :
                String.format("%g", value);
    }

    public ZoomableCachingCanvas getCanvas() {
        return this;
    }


    @Override
    public Point computeSize(int wHint, int hHint) {
        return new Point(800, 600);
    }
    @Override
    public Point computeSize(int wHint, int hHint, boolean changed) {
        return new Point(800, 600);
    }
}