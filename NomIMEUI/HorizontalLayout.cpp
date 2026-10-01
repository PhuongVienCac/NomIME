#include "stdafx.h"
#include "HorizontalLayout.h"

using namespace nomime;

void HorizontalLayout::DoLayout(CDCHandle dc, PDWR pDWR) {
  CSize size;
  int width = offsetX + real_margin_x, height = offsetY + real_margin_y;
  int w = offsetX + real_margin_x;

  /* calc mark_text sizes */
  if ((_style.hilited_mark_color & 0xff000000)) {
    CSize sg;
    if (candidates_count) {
      if (_style.mark_text.empty())
        GetTextSizeDW(L"|", 1, pDWR->pTextFormat, pDWR, &sg);
      else
        GetTextSizeDW(_style.mark_text, _style.mark_text.length(),
                      pDWR->pTextFormat, pDWR, &sg);
    }

    mark_width = sg.cx;
    mark_height = sg.cy;
    if (_style.mark_text.empty()) {
      mark_width = mark_height / 7;
      if (_style.linespacing && _style.baseline)
        mark_width =
            (int)((float)mark_width / ((float)_style.linespacing / 100.0f));
      mark_width = max(mark_width, 6);
    }
    mark_gap = (_style.mark_text.empty()) ? mark_width
                                          : mark_width + _style.hilite_spacing;
  }
  int base_offset = ((_style.hilited_mark_color & 0xff000000)) ? mark_gap : 0;

  // calc page indicator
  CSize pgszl, pgszr;
  if (!IsInlinePreedit()) {
    GetTextSizeDW(pre, pre.length(), pDWR->pPreeditTextFormat, pDWR, &pgszl);
    GetTextSizeDW(next, next.length(), pDWR->pPreeditTextFormat, pDWR, &pgszr);
  }
  bool page_en = (_style.prevpage_color & 0xff000000) &&
                 (_style.nextpage_color & 0xff000000);
  int pgw = page_en ? (pgszl.cx + pgszr.cx + _style.hilite_spacing +
                       _style.hilite_padding_x * 2)
                    : 0;
  int pgh = page_en ? max(pgszl.cy, pgszr.cy) : 0;

  /* Preedit */
  if (!IsInlinePreedit() && !_context.preedit.str.empty()) {
    size = GetPreeditSize(dc, _context.preedit, pDWR->pPreeditTextFormat, pDWR);
    int szx = pgw, szy = max(size.cy, pgh);
    // icon size higher then preedit text
    int yoffset = (STATUS_ICON_SIZE >= szy && ShouldDisplayStatusIcon())
                      ? (STATUS_ICON_SIZE - szy) / 2
                      : 0;
    _preeditRect.SetRect(w, height + yoffset, w + size.cx,
                         height + yoffset + size.cy);
    height += szy + 2 * yoffset + _style.spacing;
    width = max(width, real_margin_x * 2 + size.cx + szx);
    if (ShouldDisplayStatusIcon())
      width += STATUS_ICON_SIZE;
  }

  /* Auxiliary */
  if (!_context.aux.str.empty()) {
    size = GetPreeditSize(dc, _context.aux, pDWR->pPreeditTextFormat, pDWR);
    // icon size higher then auxiliary text
    int yoffset = (STATUS_ICON_SIZE >= size.cy && ShouldDisplayStatusIcon())
                      ? (STATUS_ICON_SIZE - size.cy) / 2
                      : 0;
    _auxiliaryRect.SetRect(w, height + yoffset, w + size.cx,
                           height + yoffset + size.cy);
    height += size.cy + 2 * yoffset + _style.spacing;
    width = max(width, real_margin_x * 2 + size.cx);
  }

  int row_cnt = 0;
  int max_width_of_rows = 0;
  int height_of_rows[MAX_CANDIDATES_COUNT] = {0};    // height of every row
  int row_of_candidate[MAX_CANDIDATES_COUNT] = {0};  // row info of every cand
  int mintop_of_rows[MAX_CANDIDATES_COUNT] = {0};
  // right edge, height of the first line (label/text) and of the whole cell
  // (first line + comment below the text) of every candidate
  int cand_right[MAX_CANDIDATES_COUNT] = {0};
  int first_h[MAX_CANDIDATES_COUNT] = {0};
  int cell_h[MAX_CANDIDATES_COUNT] = {0};
  // only when there are candidates
  if (candidates_count) {
    w = offsetX + real_margin_x;
    for (auto i = 0; i < candidates_count && i < MAX_CANDIDATES_COUNT; ++i) {
      int current_cand_width = 0;
      if (i > 0)
        w += _style.candidate_spacing;
      if (id == i)
        w += base_offset;
      /* Label */
      std::wstring label =
          GetLabelText(labels, i, _style.label_text_format.c_str());
      GetTextSizeDW(label, label.length(), pDWR->pLabelTextFormat, pDWR, &size);
      _candidateLabelRects[i].SetRect(w, height, w + size.cx * labelFontValid,
                                      height + size.cy);
      w += size.cx * labelFontValid;
      current_cand_width += size.cx * labelFontValid;

      /* Text */
      w += _style.hilite_spacing;
      const std::wstring& text = candidates.at(i).str;
      GetTextSizeDW(text, text.length(), pDWR->pTextFormat, pDWR, &size);
      int text_w = size.cx * textFontValid;
      int text_h = size.cy;
      int first_line_h = max(_candidateLabelRects[i].Height(), text_h);
      int comment_w = 0;
      int comment_h = 0;

      /* Comment: placed below the text */
      bool cmtFontNotTrans =
          (i == id && (_style.hilited_comment_text_color & 0xff000000)) ||
          (i != id && (_style.comment_text_color & 0xff000000));
      bool has_comment =
          !comments.at(i).str.empty() && cmtFontValid && cmtFontNotTrans;
      if (has_comment) {
        const std::wstring& comment = comments.at(i).str;
        GetTextSizeDW(comment, comment.length(), pDWR->pCommentTextFormat, pDWR,
                      &size);
        comment_w = size.cx * cmtFontValid;
        comment_h = size.cy * cmtFontValid;
      }
      // the text and its comment are centered on each other inside a column
      // as wide as the wider of the two
      int column_w = max(text_w, comment_w);
      int text_left = w + (column_w - text_w) / 2;
      _candidateTextRects[i].SetRect(text_left, height, text_left + text_w,
                                     height + text_h);
      if (has_comment) {
        int comment_left = w + (column_w - comment_w) / 2;
        _candidateCommentRects[i].SetRect(comment_left, height + first_line_h,
                                          comment_left + comment_w,
                                          height + first_line_h + comment_h);
      } else /* Used for highlighted candidate calculation below */
        _candidateCommentRects[i].SetRect(w, height, w, height);
      w += column_w;
      current_cand_width += (column_w + _style.hilite_spacing) * textFontValid;
      cand_right[i] = w;
      first_h[i] = first_line_h;
      cell_h[i] = first_line_h + comment_h;

      int base_left = (i == id) ? _candidateLabelRects[i].left - base_offset
                                : _candidateLabelRects[i].left;
      // if not the first candidate of current row, and current candidate's
      // right > _style.max_width
      if (_style.max_width > 0 && (base_left > real_margin_x + offsetX) &&
          (cand_right[i] - offsetX + real_margin_x > _style.max_width)) {
        // max_width_of_rows current row
        max_width_of_rows = max(max_width_of_rows, cand_right[i - 1]);
        w = offsetX + real_margin_x + (i == id ? base_offset : 0);
        int ofx = w - _candidateLabelRects[i].left;
        int ofy = height_of_rows[row_cnt] + _style.candidate_spacing;
        // offset rects to next row
        _candidateLabelRects[i].OffsetRect(ofx, ofy);
        _candidateTextRects[i].OffsetRect(ofx, ofy);
        _candidateCommentRects[i].OffsetRect(ofx, ofy);
        cand_right[i] += ofx;
        // max width of next row, if it's the last candidate, make sure
        // max_width_of_rows calc right
        max_width_of_rows = max(max_width_of_rows, cand_right[i]);
        mintop_of_rows[row_cnt] = height;
        height += ofy;
        // re calc rect position, decrease offsetX for origin
        w += current_cand_width;
        row_cnt++;
      } else
        max_width_of_rows = max(max_width_of_rows, w);
      // height of current row is the tallest cell (text with comment below)
      mintop_of_rows[row_cnt] = height;
      height_of_rows[row_cnt] = max(height_of_rows[row_cnt], cell_h[i]);
      // set row info of current candidate
      row_of_candidate[i] = row_cnt;
    }

    // reposition for alignment, exp when rect height not equal to
    // height_of_rows
    for (auto i = 0; i < candidates_count && i < MAX_CANDIDATES_COUNT; ++i) {
      int base_left = (i == id) ? _candidateLabelRects[i].left - base_offset
                                : _candidateLabelRects[i].left;
      _candidateRects[i].SetRect(base_left, mintop_of_rows[row_of_candidate[i]],
                                 cand_right[i],
                                 mintop_of_rows[row_of_candidate[i]] +
                                     height_of_rows[row_of_candidate[i]]);
      // align the label and the text inside the first line of the cell, then
      // align the whole cell (first line + comment below) inside the row
      const auto align_offset = [&](int space) {
        if (_style.align_type == UIStyle::ALIGN_CENTER)
          return space / 2;
        if (_style.align_type == UIStyle::ALIGN_BOTTOM)
          return space;
        return 0;
      };
      int cell_offset =
          align_offset(height_of_rows[row_of_candidate[i]] - cell_h[i]);
      int ol = cell_offset +
               align_offset(first_h[i] - _candidateLabelRects[i].Height());
      int ot = cell_offset +
               align_offset(first_h[i] - _candidateTextRects[i].Height());
      _candidateLabelRects[i].OffsetRect(0, ol);
      _candidateTextRects[i].OffsetRect(0, ot);
      _candidateCommentRects[i].OffsetRect(0, cell_offset);
    }
    height = mintop_of_rows[row_cnt] + height_of_rows[row_cnt] - offsetY;
    width = max(width, max_width_of_rows);
  } else {
    height -= _style.spacing + offsetY;
    width += _style.hilite_spacing + _style.border;
  }

  width += real_margin_x;
  height += real_margin_y;

  if (candidates_count) {
    width = max(width, _style.min_width);
    height = max(height, _style.min_height);
  }
  if (candidates_count) {
    for (auto i = 0; i < candidates_count && i < MAX_CANDIDATES_COUNT; ++i) {
      // make rightest candidate's rect right the same for better look
      if ((i < candidates_count - 1 &&
           row_of_candidate[i] < row_of_candidate[i + 1]) ||
          (i == candidates_count - 1))
        _candidateRects[i].right = width - real_margin_x;
    }
  }
  _highlightRect = _candidateRects[id];
  UpdateStatusIconLayout(&width, &height);
  _contentSize.SetSize(width + offsetX, height + 2 * offsetY);
  _contentRect.SetRect(0, 0, _contentSize.cx, _contentSize.cy);

  // calc page indicator
  if (page_en && candidates_count && !_style.inline_preedit) {
    int _prex = _contentSize.cx - offsetX - real_margin_x +
                _style.hilite_padding_x - pgw;
    int _prey = (_preeditRect.top + _preeditRect.bottom) / 2 - pgszl.cy / 2;
    _prePageRect.SetRect(_prex, _prey, _prex + pgszl.cx, _prey + pgszl.cy);
    _nextPageRect.SetRect(_prePageRect.right + _style.hilite_spacing, _prey,
                          _prePageRect.right + _style.hilite_spacing + pgszr.cx,
                          _prey + pgszr.cy);
    if (ShouldDisplayStatusIcon()) {
      _prePageRect.OffsetRect(-STATUS_ICON_SIZE, 0);
      _nextPageRect.OffsetRect(-STATUS_ICON_SIZE, 0);
    }
  }

  // prepare temp rect _bgRect for roundinfo calculation
  CopyRect(_bgRect, _contentRect);
  _bgRect.DeflateRect(offsetX + 1, offsetY + 1);
  // prepare round info for single row status, only for single row situation
  _PrepareRoundInfo(dc);
  // readjust for multi rows
  if (row_cnt)  // row_cnt > 0, at least 2 candidates
  {
    _roundInfo[0].IsBottomLeftNeedToRound = false;
    _roundInfo[candidates_count - 1].IsTopRightNeedToRound = false;
    for (auto i = 1; i < candidates_count; i++) {
      _roundInfo[i].Hemispherical = _roundInfo[0].Hemispherical;
      if (row_of_candidate[i] == row_cnt &&
          row_of_candidate[i - 1] == row_cnt - 1)
        _roundInfo[i].IsBottomLeftNeedToRound = true;
      if (row_of_candidate[i] == 0 && row_of_candidate[i + 1] == 1)
        _roundInfo[i].IsTopRightNeedToRound = _style.inline_preedit;
    }
  }
  // truely draw content size calculation
  _contentRect.DeflateRect(offsetX, offsetY);
}
