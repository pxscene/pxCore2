// pxCore CopyRight 2007-2009 John Robinson
// Portable Framebuffer and Windowing Library
// pxRect.h

#ifndef PX_RECT_H
#define PX_RECT_H

// A class used to describe a rectangle using integer coordinates
class pxRect
{
public:
    inline pxRect(): mLeft(0), mTop(0), mRight(0), mBottom(0)
    {
    }

    inline pxRect(int l, int t, int r, int b)
    {
        mLeft = l;
        mTop = t;
        mRight = r;
        mBottom = b;
    }

    inline int width() const
    {
        return mRight-mLeft;
    }

    // set right = left + w;
    inline void setWidth(int w)
    {
        mRight = mLeft + w;
    }

    inline int height() const
    {
        return mBottom-mTop;
    }

    // set bottom = top + h
    inline void setHeight(int h)
    {
        mBottom = mBottom + h;
    }

    inline int left() const
    {
        return mLeft;
    }

    inline int top() const
    {
        return mTop;
    }

    inline int right() const
    {
        return mRight;
    }

    inline int bottom() const
    {
        return mBottom;
    }

    inline void setLeft(int l)
    {
        mLeft = l;
    }

    inline void setTop(int t)
    {
        mTop = t;
    }

    inline void setBottom(int b)
    {
        mBottom = b;
    }

    inline void setRight(int r)
    {
        mRight = r;
    }

    inline void setLTRB(int l, int t, int r, int b)
    {
        mLeft = l;
        mTop = t;
        mRight = r;
        mBottom = b;
    }

    inline void setLTWH(int l, int t, int w, int h)
    {
        mLeft = l;
        mTop = t;
        mRight = l + w;
        mBottom = t + h;
    }

    inline void set(int l, int t, int r, int b)
    {
        setLTRB(l, t, r, b);
    }

    inline void setEmpty()
    {
        setLTRB(0, 0, 0, 0);
    }

    void intersect(const pxRect& r)
    {
        mLeft = pxMax<int>(mLeft, r.mLeft);
        mTop = pxMax<int>(mTop, r.mTop);
        mRight = pxMin<int>(mRight, r.mRight);
        mBottom = pxMin<int>(mBottom, r.mBottom);
#if 0
        if (mRight < mLeft)
            mRight = mLeft;
        if (mBottom < mTop)
            mBottom = mTop;
#endif
    }

    void unionRect(const pxRect& r)
    {
        if (!r.isEmpty())
        {
            if (!isEmpty())
            {
                mLeft = pxMin<int>(mLeft, r.mLeft);
                mTop = pxMin<int>(mTop, r.mTop);
                mRight = pxMax<int>(mRight, r.mRight);
                mBottom = pxMax<int>(mBottom, r.mBottom);
            }
            else
                *this = r;
        }
    }

    bool hitTest(int x, int y)
    {
        return (x >= mLeft && x <= mRight && y >= mTop && y <= mBottom);
    }

    bool isEmpty() const
    {
        return (mLeft >= mRight) || (mTop >= mBottom);
    }

private:
    int mLeft, mTop, mRight, mBottom;
};


#endif

