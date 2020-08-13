package com.example.textrendering

import android.content.res.AssetManager
import android.graphics.*
import android.os.Bundle
import android.util.Log
import android.util.TypedValue
import android.view.*
import android.widget.TextView
import androidx.annotation.Keep

import androidx.appcompat.app.AppCompatActivity
import kotlin.math.abs
import kotlin.math.ceil

class MainActivity : AppCompatActivity(), SurfaceHolder.Callback
{
    private var surfaceView:SurfaceView? = null;

    private lateinit var assetManager:AssetManager;


    override fun onCreate( savedInstanceState:Bundle? )
    {
        super.onCreate( savedInstanceState );
        setContentView( R.layout.activity_main );

        if( surfaceView == null )
            surfaceView = findViewById( R.id.surface_view );

        surfaceView!!.holder.addCallback( this );

        assetManager = resources.assets;
        init( assetManager );
    }

    override fun onStart()
    {
        super.onStart();
        start();
    }

    override fun onStop()
    {
        super.onStop();
        stop();
    }

    override fun onDestroy()
    {
        super.onDestroy();
        destroy();

        surfaceView!!.holder.removeCallback( this );
    }


    override fun surfaceCreated( holder:SurfaceHolder ) {}

    override fun surfaceChanged( holder:SurfaceHolder, format:Int, width:Int, height:Int )
    {
        var surface:Surface = holder.surface;
        surfaceChanged( surface, width, height );
    }

    override fun surfaceDestroyed( holder:SurfaceHolder )
    {
        surfaceDestroyed();
    }


    fun getTypeface( fontFilename:String ) : Typeface
    {
        var result:Typeface = Typeface.DEFAULT;
        
        if( fontFilename != "" )
            result = Typeface.createFromAsset( assetManager, fontFilename );

        return result;
    }


    @Keep
    public fun getTextSize( string:String,
                            fontFilename:String,
                            fontSize:Int,
                            kerning:Int,
                            hAlignment:Int,
                            textSize:IntArray )
    {
        var textView:TextView = TextView( this );
        textView.layout( 0, 0, textSize[ 0 ], textSize[ 1 ] );
        textView.setTextSize( TypedValue.COMPLEX_UNIT_SP, fontSize.toFloat() );
        textView.typeface = getTypeface( fontFilename );

        //@NOTE letterSpacing in ems, ems = textSize in pixels
        textView.letterSpacing = kerning.toFloat() / textView.textSize;

        if( hAlignment == 0 )
            textView.gravity = Gravity.LEFT;
        else if( hAlignment == 1 )
            textView.gravity = Gravity.CENTER_HORIZONTAL;
        else if( hAlignment == 2 )
            textView.gravity = Gravity.RIGHT;

        textView.text = string;

        var widthMeasureSpec:Int = View.MeasureSpec.makeMeasureSpec( textSize[ 0 ], View.MeasureSpec.AT_MOST );
        var heightMeasureSpec:Int = View.MeasureSpec.makeMeasureSpec( 0, View.MeasureSpec.UNSPECIFIED );
        textView.measure( widthMeasureSpec, heightMeasureSpec );
        textSize[ 0 ] = textView.measuredWidth;
        textSize[ 1 ] = textView.measuredHeight;
    }

    @Keep
    public fun getTextBitmap( string:String,
                                fontFilename:String,
                                fontSize:Int,
                                kerning:Int,
                                hAlignment:Int,
                                bitmapWidth:Int,
                                bitmapHeight:Int,
                                textSize:IntArray,
                                pixels:IntArray )
    {
        var textView:TextView = TextView( this );
        textView.layout( 0, 0, textSize[ 0 ], textSize[ 1 ] );
        textView.setTextSize( TypedValue.COMPLEX_UNIT_SP, fontSize.toFloat() );
        textView.setTextColor( Color.WHITE );
        textView.typeface = getTypeface( fontFilename );

        //@NOTE letterSpacing in ems, ems = textSize in pixels
        textView.letterSpacing = kerning.toFloat() / textView.textSize;

        if( hAlignment == 0 )
            textView.gravity = Gravity.LEFT;
        else if( hAlignment == 1 )
            textView.gravity = Gravity.CENTER_HORIZONTAL;
        else if( hAlignment == 2 )
            textView.gravity = Gravity.RIGHT;

        textView.text = string;

        var bitmap:Bitmap = Bitmap.createBitmap( bitmapWidth, bitmapHeight, Bitmap.Config.ARGB_8888 );
        bitmap.eraseColor( 0x00000000 );

        var canvas:Canvas = Canvas();
        canvas.setBitmap( bitmap );

        textView.draw( canvas );

        bitmap.getPixels( pixels, 0, bitmapWidth, 0, 0, bitmapWidth, bitmapHeight );
        bitmap.recycle();
    }

    @Keep
    public fun getFontAtlasTextSize( string:String,
                                        fontFilename:String,
                                        fontSize:Int,
                                        ascent:IntArray,
                                        descent:IntArray,
                                        glyphWidth:ShortArray,
                                        glyphHeight:ShortArray )
    {
        var paint:Paint = Paint();
        paint.textAlign = Paint.Align.LEFT;
        paint.isAntiAlias = true;
        paint.setARGB( 0xff, 0xff, 0xff, 0xff );
        paint.textSize = fontSize * resources.displayMetrics.scaledDensity;
        paint.typeface = getTypeface( fontFilename );

        ascent[ 0 ] = -paint.fontMetricsInt.ascent;
        descent[ 0 ] = -paint.fontMetricsInt.descent;

        for( i in string.indices )
        {
            glyphWidth[ i ] = ceil( paint.measureText( string, i, i + 1 ) ).toShort();
            glyphHeight[ i ] = ceil( abs( paint.ascent() ) + paint.descent() ).toShort();
        }
    }

    @Keep
    public fun getFontAtlasBitmap( string:String,
                                    fontFilename:String,
                                    fontSize:Int,
                                    bitmapWidth:Int,
                                    bitmapHeight:Int,
                                    glyphAtlasX:ShortArray,
                                    glyphAtlasY:ShortArray,
                                    pixels:IntArray )
    {
        var canvas:Canvas = Canvas();

        var paint:Paint = Paint();
        paint.textAlign = Paint.Align.LEFT;
        paint.isAntiAlias = true;
        paint.setARGB( 0xff, 0xff, 0xff, 0xff );
        paint.textSize = fontSize * resources.displayMetrics.scaledDensity;
        paint.typeface = getTypeface( fontFilename );

        var bitmap:Bitmap = Bitmap.createBitmap( bitmapWidth, bitmapHeight, Bitmap.Config.ARGB_8888 );
        bitmap.eraseColor( 0x00000000 );
        canvas.setBitmap( bitmap );

        var height:Float = ceil( abs( paint.ascent() ) + paint.descent() );

        for( i in string.indices )
            canvas.drawText( string[ i ].toString(), glyphAtlasX[ i ].toFloat(), glyphAtlasY[ i ] + height - ceil( paint.descent() ), paint );

        bitmap.getPixels( pixels, 0, bitmapWidth, 0, 0, bitmapWidth, bitmapHeight );
        bitmap.recycle();
    }


    external fun init( assetManager:AssetManager );
    external fun start();
    external fun stop();
    external fun destroy();

    external fun surfaceChanged( surface:Surface, width:Int, height:Int );
    external fun surfaceDestroyed();
    

    companion object
    {
        init
        {
            System.loadLibrary( "main-lib" )
        }
    }
}
