﻿/*
 * (c) Copyright Ascensio System SIA 2010-2023
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation. In accordance with
 * Section 7(a) of the GNU AGPL its Section 15 shall be amended to the effect
 * that Ascensio System SIA expressly excludes the warranty of non-infringement
 * of any third-party rights.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE. For
 * details, see the GNU AGPL at: http://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA at 20A-6 Ernesta Birznieka-Upish
 * street, Riga, Latvia, EU, LV-1050.
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * Pursuant to Section 7(b) of the License you must retain the original Product
 * logo when distributing the program. Pursuant to Section 7(e) we decline to
 * grant you any rights under trademark law for use of our trademarks.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 */
#include "ConditionalFormatting.h"

#include "../../XlsbFormat/Biff12_unions/CONDITIONALFORMATTING.h"
#include "../../XlsbFormat/Biff12_unions/CONDITIONALFORMATTING14.h"
#include "../../XlsbFormat/Biff12_unions/CFRULE.h"
#include "../../XlsbFormat/Biff12_unions/CFRULE14.h"
#include "../../XlsbFormat/Biff12_unions/uCFVO.h"
#include "../../XlsbFormat/Biff12_unions/uCFVO14.h"
#include "../../XlsbFormat/Biff12_unions/COLORSCALE14.h"
#include "../../XlsbFormat/Biff12_unions/COLORSCALE.h"
#include "../../XlsbFormat/Biff12_unions/DATABAR.h"
#include "../../XlsbFormat/Biff12_unions/DATABAR14.h"
#include "../../XlsbFormat/Biff12_unions/ICONSET.h"
#include "../../XlsbFormat/Biff12_unions/ICONSET14.h"
#include "../../XlsbFormat/Biff12_unions/FRTCFRULE.h"

#include "../../XlsbFormat/Biff12_records/BeginConditionalFormatting.h"
#include "../../XlsbFormat/Biff12_records/BeginConditionalFormatting14.h"
#include "../../XlsbFormat/Biff12_records/FRTBegin.h"
#include "../../XlsbFormat/Biff12_records/BeginCFRule.h"
#include "../../XlsbFormat/Biff12_records/BeginCFRule14.h"
#include "../../XlsbFormat/Biff12_records/BeginDatabar.h"
#include "../../XlsbFormat/Biff12_records/BeginDatabar14.h"
#include "../../XlsbFormat/Biff12_records/BeginIconSet.h"
#include "../../XlsbFormat/Biff12_records/BeginIconSet14.h"

#include "../../XlsbFormat/Biff12_structures/CFOper.h"
#include "../../XlsbFormat/Biff12_structures/CFTextOper.h"
#include "../../XlsbFormat/Biff12_structures/CFDateOper.h"
#include "../../XlsbFormat/Biff12_structures/CFVOtype.h"

#include "../../XlsbFormat/Biff12_records/CFVO.h"
#include "../../XlsbFormat/Biff12_records/CFRuleExt.h"
#include "../../XlsbFormat/Biff12_records/CFVO14.h"
#include "../../XlsbFormat/Biff12_records/Color14.h"
#include "../../XlsbFormat/Biff12_records/CFIcon.h"

#include "../../DocxFormat/Drawing/DrawingExt.h"
#include "../../Common/SimpleTypes_Shared.h"
//#include "../../Common/SimpleTypes_Spreadsheet.h"
#include "../ComplexTypes_Spreadsheet.h"

#include "../Styles/dxf.h"
#include "../Styles/Colors.h"
#include "../Styles/Xfs.h"
#include "../Styles/NumFmts.h"
#include "../Styles/Fonts.h"
#include "../Styles/Fills.h"
#include "../Styles/Borders.h"

#include "../../../MsBinaryFile/XlsFile/Format/Binary/CFStreamCacheWriter.h"

using namespace XLS;

namespace OOX
{
namespace Spreadsheet
{
CFormulaCF::CFormulaCF()
{
}
CFormulaCF::~CFormulaCF()
{
}
void CFormulaCF::fromXML(XmlUtils::CXmlNode& node)
{
}
std::wstring CFormulaCF::toXML() const
{
	return L"";
}
void CFormulaCF::toXML(NSStringUtils::CStringBuilder& writer) const
{
	toXML2(writer, false);
}
EElementType CFormulaCF::getType () const
{
	return et_x_FormulaCF;
}
void CFormulaCF::toXML2(NSStringUtils::CStringBuilder& writer, bool bExtendedWrite) const
{
	std::wstring node_name = bExtendedWrite ? L"xm:f" : L"formula";

	writer.WriteString(L"<" + node_name + L">");
	writer.WriteEncodeXmlString(m_sText);
	writer.WriteString(L"</" + node_name + L">");
}
void CFormulaCF::fromXML(XmlUtils::CXmlLiteReader& oReader)
{
	m_sNodeName = oReader.GetName();

	if (oReader.IsEmptyNode())
		return;

	m_sText = oReader.GetText3();
}
const CFormulaCF CFormulaCF::Merge(const CFormulaCF& oPrev, const CFormulaCF& oCurrent)
{
	CFormulaCF oFormula;

	if (oCurrent.m_sText.empty() == false)
	{
		oFormula.m_sText = oCurrent.m_sText;
		oFormula.m_sNodeName = oCurrent.m_sNodeName;
	}
	else
	{
		oFormula.m_sText = oPrev.m_sText;
		oFormula.m_sNodeName = oPrev.m_sNodeName;
	}

	return oFormula;
}
bool CFormulaCF::isExtended ()
{
	return (m_sNodeName == L"xm:f");
}

CConditionalFormatValueObject::CConditionalFormatValueObject()
{
}
CConditionalFormatValueObject::~CConditionalFormatValueObject()
{
}
void CConditionalFormatValueObject::fromXML(XmlUtils::CXmlNode& node)
{
}
std::wstring CConditionalFormatValueObject::toXML() const
{
	return L"";
}
void CConditionalFormatValueObject::toXML(NSStringUtils::CStringBuilder& writer) const
{
	toXML2(writer, false);
}
EElementType CConditionalFormatValueObject::getType () const
{
	return et_x_ConditionalFormatValueObject;
}
bool CConditionalFormatValueObject::isExtended ()
{
	return (m_oFormula.IsInit());
}
void CConditionalFormatValueObject::ReadAttributes(XmlUtils::CXmlLiteReader& oReader)
{
	WritingElement_ReadAttributes_Start(oReader)
		WritingElement_ReadAttributes_Read_if		(oReader, L"gte"	, m_oGte)
		WritingElement_ReadAttributes_Read_else_if	(oReader, L"type"	, m_oType)
		WritingElement_ReadAttributes_Read_else_if	(oReader, L"val"	, m_oVal)
	WritingElement_ReadAttributes_End(oReader)
}
void CConditionalFormatValueObject::toXML2(NSStringUtils::CStringBuilder& writer, bool bExtendedWrite) const
{
	if (false == m_oType.IsInit()) return;

	if (bExtendedWrite == false)
	{
		if (m_oType->GetValue() == SimpleTypes::Spreadsheet::autoMin) m_oType->SetValue(SimpleTypes::Spreadsheet::Minimum);
		if (m_oType->GetValue() == SimpleTypes::Spreadsheet::autoMax) m_oType->SetValue(SimpleTypes::Spreadsheet::Maximum);
	}

	std::wstring node_name = bExtendedWrite ? L"x14:cfvo" : L"cfvo";

	writer.WriteString(L"<" + node_name);
		WritingStringAttrString(L"type", m_oType->ToString());
		if (m_oGte.IsInit() && false == m_oGte->ToBool())
		{
			writer.WriteString(L" gte=\"0\"");
		}
		if (!bExtendedWrite)
		{
			if (m_oVal.IsInit())
			{
				WritingStringAttrEncodeXmlString(L"val", m_oVal.get());
			}
			else if (m_oFormula.IsInit())
			{
				WritingStringAttrEncodeXmlString(L"val", m_oFormula->m_sText);
			}

		}
	writer.WriteString(L">");

	if (bExtendedWrite)
	{
		if (true == m_oFormula.IsInit())
		{
			m_oFormula->toXML2(writer, true);
		}
		else if (m_oVal.IsInit())
		{
			CFormulaCF formla; formla.m_sText = m_oVal.get();
			formla.toXML2(writer, true);
		}

	}

	writer.WriteString(L"</" + node_name + L">");
}
void CConditionalFormatValueObject::fromXML(XmlUtils::CXmlLiteReader& oReader)
{
	ReadAttributes(oReader);

	if (oReader.IsEmptyNode())
		return;

	int nCurDepth = oReader.GetDepth();
	while (oReader.ReadNextSiblingNode(nCurDepth))
	{
		std::wstring sName = XmlUtils::GetNameNoNS(oReader.GetName());
		if (L"formula" == sName || L"f" == sName)
			m_oFormula = oReader;
	}
}
XLS::BaseObjectPtr CConditionalFormatValueObject::toBin(bool isIcon)
{
    auto ptr(new XLSB::uCFVO);
    XLS::BaseObjectPtr objectPtr(ptr);

    auto ptr1(new XLSB::CFVO);
    ptr->m_BrtCFVO = XLS::BaseObjectPtr{ptr1};
    ptr1->fSaveGTE = isIcon;
    if(m_oGte.IsInit())
        ptr1->fGTE = m_oGte->GetValue();
    else
        ptr1->fGTE = true;
    if (m_oType == SimpleTypes::Spreadsheet::ECfvoType::Number)
        ptr1->iType = XLSB::CFVOtype::CFVONUM;
    else if (m_oType == SimpleTypes::Spreadsheet::ECfvoType::Minimum)
        ptr1->iType = XLSB::CFVOtype::CFVOMIN;
    else if (m_oType == SimpleTypes::Spreadsheet::ECfvoType::Maximum)
        ptr1->iType = XLSB::CFVOtype::CFVOMAX;
    else if (m_oType == SimpleTypes::Spreadsheet::ECfvoType::Percent)
        ptr1->iType = XLSB::CFVOtype::CFVOPERCENT;
    else if (m_oType == SimpleTypes::Spreadsheet::ECfvoType::Percentile)
        ptr1->iType = XLSB::CFVOtype::CFVOPERCENTILE;
    else if (m_oType == SimpleTypes::Spreadsheet::ECfvoType::Formula)
        ptr1->iType = XLSB::CFVOtype::CFVOFMLA;
    else
        ptr1->iType = XLSB::CFVOtype::CFVONUM;

    if(m_oVal.IsInit() && ptr1->iType != XLSB::CFVOtype::CFVOFMLA)
        try
        {
            ptr1->numParam.data.value = std::stod(m_oVal.get());
        }
        catch (std::exception)
        {
            ptr1->formula = m_oVal.get();
            auto lastValType = GETBITS(ptr1->formula.rgce.sequence.rbegin()->get()->ptg_id.get(),5,6);
            if(lastValType == 1 || lastValType == 3)
            {
                SETBITS(ptr1->formula.rgce.sequence.rbegin()->get()->ptg_id.get(),5,6,2);
            }

            ptr1->cbFmla = 1;
        }
    if(ptr1->iType.get_type() == XLSB::CFVOtype::CFVOMIN)
        ptr1->numParam.data.value = 0;
    else if(ptr1->iType.get_type() == XLSB::CFVOtype::CFVOMAX)
        ptr1->numParam.data.value = 0;

    if(static_cast<_UINT32>(ptr1->iType) == XLSB::CFVOtype::CFVOFMLA && m_oVal.IsInit())
    {
        ptr1->formula = m_oVal.get();
        auto lastValType = GETBITS(ptr1->formula.rgce.sequence.rbegin()->get()->ptg_id.get(),5,6);
        if(lastValType == 1 || lastValType == 3)
        {
            SETBITS(ptr1->formula.rgce.sequence.rbegin()->get()->ptg_id.get(),5,6,2);
        }

        ptr1->cbFmla = 1;
    }
    else
    {
        ptr1->cbFmla = 0;
    }

    return objectPtr;
}
void CConditionalFormatValueObject::toBin(XLS::StreamCacheWriterPtr& writer, const bool isIcon)
{
    XLSB::CFVO cfvo;
    cfvo.fSaveGTE = isIcon;
    if(m_oGte.IsInit())
        cfvo.fGTE = m_oGte->GetValue();
    else
        cfvo.fGTE = true;

    auto valType = m_oType->GetValue();
    if (valType == SimpleTypes::Spreadsheet::ECfvoType::Number)
        cfvo.iType = XLSB::CFVOtype::CFVONUM;
    else if (valType == SimpleTypes::Spreadsheet::ECfvoType::Minimum || valType == SimpleTypes::Spreadsheet::ECfvoType::autoMin)
        cfvo.iType = XLSB::CFVOtype::CFVOMIN;
    else if (valType == SimpleTypes::Spreadsheet::ECfvoType::Maximum || valType == SimpleTypes::Spreadsheet::ECfvoType::autoMax)
        cfvo.iType = XLSB::CFVOtype::CFVOMAX;
    else if (valType == SimpleTypes::Spreadsheet::ECfvoType::Percent)
        cfvo.iType = XLSB::CFVOtype::CFVOPERCENT;
    else if (valType == SimpleTypes::Spreadsheet::ECfvoType::Percentile)
        cfvo.iType = XLSB::CFVOtype::CFVOPERCENTILE;
    else if (valType == SimpleTypes::Spreadsheet::ECfvoType::Formula)
        cfvo.iType = XLSB::CFVOtype::CFVOFMLA;
    else
        cfvo.iType = XLSB::CFVOtype::CFVONUM;
    if(valType == SimpleTypes::Spreadsheet::ECfvoType::Formula && !(m_oVal.IsInit()) && m_oFormula.IsInit())
        m_oVal = m_oFormula->m_sText;
    if(m_oVal.IsInit() && cfvo.iType != XLSB::CFVOtype::CFVOFMLA)
        try
        {
            cfvo.numParam.data.value = std::stod(m_oVal.get());
        }
        catch (std::exception)
        {
            cfvo.formula = m_oVal.get();
            auto lastValType = GETBITS(cfvo.formula.rgce.sequence.rbegin()->get()->ptg_id.get(),5,6);
            if(lastValType == 1 || lastValType == 3)
            {
                SETBITS(cfvo.formula.rgce.sequence.rbegin()->get()->ptg_id.get(),5,6,2);
            }

            cfvo.cbFmla = 1;
        }
    if(cfvo.iType.get_type() == XLSB::CFVOtype::CFVOMIN)
        cfvo.numParam.data.value = 0;
    else if(cfvo.iType.get_type() == XLSB::CFVOtype::CFVOMAX)
        cfvo.numParam.data.value = 0;

    if(static_cast<_UINT32>(cfvo.iType) == XLSB::CFVOtype::CFVOFMLA && m_oVal.IsInit())
    {
        cfvo.formula = m_oVal.get();
        auto lastValType = GETBITS(cfvo.formula.rgce.sequence.rbegin()->get()->ptg_id.get(),5,6);
        if(lastValType == 1 || lastValType == 3)
        {
            SETBITS(cfvo.formula.rgce.sequence.rbegin()->get()->ptg_id.get(),5,6,2);
        }

        cfvo.cbFmla = 1;
    }
    else
    {
        cfvo.cbFmla = 0;
    }
    cfvo.write(writer, nullptr);
}
XLS::BaseObjectPtr CConditionalFormatValueObject::toBin14(bool isIcon)
{
    auto ptr(new XLSB::uCFVO14);
    XLS::BaseObjectPtr objectPtr(ptr);

    auto ptr1(new XLSB::CFVO14);
    ptr->m_BrtCFVO14 = XLS::BaseObjectPtr{ptr1};
    ptr1->fSaveGTE = isIcon;
    if(m_oGte.IsInit())
        ptr1->fGTE = m_oGte->GetValue();
    else
        ptr1->fGTE = true;
    if (m_oType == SimpleTypes::Spreadsheet::ECfvoType::Number)
        ptr1->iType = XLSB::CFVOtype::CFVONUM;
    else if (m_oType == SimpleTypes::Spreadsheet::ECfvoType::Minimum)
        ptr1->iType = XLSB::CFVOtype::CFVOMIN;
    else if (m_oType == SimpleTypes::Spreadsheet::ECfvoType::Maximum)
        ptr1->iType = XLSB::CFVOtype::CFVOMAX;
    else if (m_oType == SimpleTypes::Spreadsheet::ECfvoType::Percent)
        ptr1->iType = XLSB::CFVOtype::CFVOPERCENT;
    else if (m_oType == SimpleTypes::Spreadsheet::ECfvoType::Percentile)
        ptr1->iType = XLSB::CFVOtype::CFVOPERCENTILE;
    else if (m_oType == SimpleTypes::Spreadsheet::ECfvoType::Formula)
        ptr1->iType = XLSB::CFVOtype::CFVOFMLA;
    else if (m_oType == SimpleTypes::Spreadsheet::ECfvoType::autoMin)
        ptr1->iType = XLSB::CFVOType14::CFVOAUTOMIN_14;
    else if (m_oType == SimpleTypes::Spreadsheet::ECfvoType::autoMax)
        ptr1->iType = XLSB::CFVOType14::CFVOAUTOMAX_14;
    else
        ptr1->iType = XLSB::CFVOtype::CFVONUM;

    if(ptr1->iType.get_type() == XLSB::CFVOtype::CFVOMIN)
        ptr1->numParam.data.value = 0;
    else if(ptr1->iType.get_type() == XLSB::CFVOtype::CFVOMAX)
        ptr1->numParam.data.value = 0;
    if(static_cast<_UINT32>(ptr1->iType) == XLSB::CFVOtype::CFVOFMLA && !(m_oVal.IsInit()) && m_oFormula.IsInit() && !m_oFormula->m_sText.empty())
        m_oVal = m_oFormula->m_sText;
    if(static_cast<_UINT32>(ptr1->iType) == XLSB::CFVOtype::CFVOFMLA && m_oVal.IsInit())
    {
        XLSB::FRTFormula tempFmla;
        tempFmla.formula = m_oVal.get();
        auto lastValType = GETBITS(tempFmla.formula.rgce.sequence.rbegin()->get()->ptg_id.get(),5,6);
        if(lastValType == 1 || lastValType == 3)
        {
            SETBITS(tempFmla.formula.rgce.sequence.rbegin()->get()->ptg_id.get(),5,6,2);
        }
        ptr1->FRTheader.rgFormulas.array.push_back(tempFmla);
        ptr1->FRTheader.fFormula = true;
        ptr1->cbFmla = 1;
    }
    else
    {
        ptr1->cbFmla = 0;
    }
    if(ptr1->cbFmla == 0 && m_oFormula.IsInit() && !m_oFormula->m_sText.empty())
    {
        try
        {
            ptr1->numParam.data.value = std::stod(m_oFormula->m_sText);
        }
        catch(std::exception)
        {}
    }
    return objectPtr;
}
void CConditionalFormatValueObject::fromBin(XLS::BaseObjectPtr& obj)
{
    ReadAttributes(obj);
}
void CConditionalFormatValueObject::ReadAttributes(XLS::BaseObjectPtr& obj)
{
    if(obj->get_type() == XLS::typeuCFVO)
    {
        auto ptr = static_cast<XLSB::uCFVO*>(obj.get());

        if(ptr != nullptr)
        {
            auto ptr1 = static_cast<XLSB::CFVO*>(ptr->m_BrtCFVO.get());
            if(ptr1 != nullptr)
            {
                m_oGte  = (bool)ptr1->fGTE;
                switch (ptr1->iType.value().get())
                {
                    case XLSB::CFVOtype::CFVONUM:
                        m_oType     = SimpleTypes::Spreadsheet::ECfvoType::Number;
                        break;
                    case XLSB::CFVOtype::CFVOMIN:
                        m_oType     = SimpleTypes::Spreadsheet::ECfvoType::Minimum;
                        break;
                    case XLSB::CFVOtype::CFVOMAX:
                        m_oType     = SimpleTypes::Spreadsheet::ECfvoType::Maximum;
                        break;
                    case XLSB::CFVOtype::CFVOPERCENT:
                        m_oType     = SimpleTypes::Spreadsheet::ECfvoType::Percent;
                        break;
                    case XLSB::CFVOtype::CFVOPERCENTILE:
                        m_oType     = SimpleTypes::Spreadsheet::ECfvoType::Percentile;
                        break;
                    case XLSB::CFVOtype::CFVOFMLA:
                        m_oType     = SimpleTypes::Spreadsheet::ECfvoType::Formula;
                        break;
                }
                m_oVal = std::to_wstring(ptr1->numParam.data.value);
                if(ptr1->cbFmla)
                {
                    m_oFormula.Init();
                    m_oFormula->m_sNodeName = L"formula";
                    m_oFormula->m_sText = ptr1->formula.getAssembledFormula();
                }
            }

        }
    }
    else if(obj->get_type() == XLS::typeuCFVO14)
    {
        auto ptr = static_cast<XLSB::uCFVO14*>(obj.get());

        if(ptr != nullptr)
        {
            auto ptr1 = static_cast<XLSB::CFVO14*>(ptr->m_BrtCFVO14.get());
            if(ptr1 != nullptr)
            {
                m_oGte  = (bool)ptr1->fGTE;
                switch (ptr1->iType.value().get())
                {
                    case XLSB::CFVOType14::CFVONUM_14:
                        m_oType     = SimpleTypes::Spreadsheet::ECfvoType::Number;
                        break;
                    case XLSB::CFVOType14::CFVOMIN_14:
                        m_oType     = SimpleTypes::Spreadsheet::ECfvoType::Minimum;
                        break;
                    case XLSB::CFVOType14::CFVOMAX_14:
                        m_oType     = SimpleTypes::Spreadsheet::ECfvoType::Maximum;
                        break;
                    case XLSB::CFVOType14::CFVOPERCENT_14:
                        m_oType     = SimpleTypes::Spreadsheet::ECfvoType::Percent;
                        break;
                    case XLSB::CFVOType14::CFVOPERCENTILE_14:
                        m_oType     = SimpleTypes::Spreadsheet::ECfvoType::Percentile;
                        break;
                    case XLSB::CFVOType14::CFVOFMLA_14:
                        m_oType     = SimpleTypes::Spreadsheet::ECfvoType::Formula;
                        break;
                    case XLSB::CFVOType14::CFVOAUTOMIN_14:
                        m_oType     = SimpleTypes::Spreadsheet::ECfvoType::autoMin;
                        break;
                    case XLSB::CFVOType14::CFVOAUTOMAX_14:
                        m_oType     = SimpleTypes::Spreadsheet::ECfvoType::autoMax;
                        break;
                }
                m_oVal = std::to_wstring(ptr1->numParam.data.value);
                if(ptr1->cbFmla && ptr1->FRTheader.rgFormulas.array.size() > 1)
                {
                    m_oFormula.Init();
                    m_oFormula->m_sNodeName = L"formula";
                    m_oFormula->m_sText = ptr1->FRTheader.rgFormulas.array[0].formula.getAssembledFormula();
                }
            }

        }
    }
}

CConditionalFormatIconSet::CConditionalFormatIconSet()
{
}
CConditionalFormatIconSet::~CConditionalFormatIconSet()
{
}
void CConditionalFormatIconSet::fromXML(XmlUtils::CXmlNode& node)
{
}
std::wstring CConditionalFormatIconSet::toXML() const
{
	return L"";
}
void CConditionalFormatIconSet::toXML(NSStringUtils::CStringBuilder& writer) const
{
	toXML2(writer, false);
}
EElementType CConditionalFormatIconSet::getType () const
{
	return et_x_ConditionalFormatIconSet;
}
void CConditionalFormatIconSet::toXML2(NSStringUtils::CStringBuilder& writer, bool bExtendedWrite) const
{
	if (!bExtendedWrite) return;

	writer.WriteString(L"<x14:cfIcon");
		WritingStringNullableAttrString(L"iconSet", m_oIconSet, m_oIconSet->ToString())
		WritingStringNullableAttrInt(L"iconId", m_oIconId, m_oIconId->GetValue())
	writer.WriteString(L"/>");
}
void CConditionalFormatIconSet::fromXML(XmlUtils::CXmlLiteReader& oReader)
{
	ReadAttributes(oReader);

    if (!oReader.IsEmptyNode())
        oReader.ReadTillEnd();
}
void CConditionalFormatIconSet::fromBin(XLS::BaseObjectPtr& obj)
{
    ReadAttributes(obj);
}
void CConditionalFormatIconSet::ReadAttributes(XmlUtils::CXmlLiteReader& oReader)
{
	WritingElement_ReadAttributes_Start(oReader)

	WritingElement_ReadAttributes_Read_if		(oReader, L"iconSet", m_oIconSet)
	WritingElement_ReadAttributes_Read_else_if	(oReader, L"iconId"	, m_oIconId)

	WritingElement_ReadAttributes_End(oReader)
}
void CConditionalFormatIconSet::ReadAttributes(XLS::BaseObjectPtr& obj)
{
    auto ptr = static_cast<XLSB::CFIcon*>(obj.get());

    if(ptr != nullptr)
    {
        if(ptr->index < 0)
             ptr->index = 0;
        m_oIconId = ptr->index;

        switch(ptr->iconSet.set)
        {
            case KPISets14::KPINIL_14:
            {
                m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::NoIcons;
                break;
            }
            case KPISets14::KPI3ARROWS_14:
            {
                m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Arrows3;
                break;
            }
            case KPISets14::KPI3ARROWSGRAY_14:
            {
                m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Arrows3Gray;
                break;
            }
            case KPISets14::KPI3FLAGS_14:
            {
                m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Flags3;
                break;
            }
            case KPISets14::KPI3TRAFFICLIGHTS1_14:
            {
                m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Traffic3Lights1;
                break;
            }
            case KPISets14::KPI3TRAFFICLIGHTS2_14:
            {
                m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Traffic3Lights2;
                break;
            }
            case KPISets14::KPI3SIGNS_14:
            {
                m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Signs3;
                break;
            }
            case KPISets14::KPI3SYMBOLS_14:
            {
                m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Symbols3;
                break;
            }
            case KPISets14::KPI3SYMBOLS2_14:
            {
                m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Symbols3_2;
                break;
            }
            case KPISets14::KPI4ARROWS_14:
            {
                m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Arrows4;
                break;
            }
            case KPISets14::KPI4ARROWSGRAY_14:
            {
                m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Arrows4Gray;
                break;
            }
            case KPISets14::KPI4REDTOBLACK_14:
            {
                m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::RedToBlack4;
                break;
            }
            case KPISets14::KPI4RATING_14:
            {
                m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Rating4;
                break;
            }
            case KPISets14::KPI4TRAFFICLIGHTS_14:
            {
                m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Traffic4Lights;
                break;
            }
            case KPISets14::KPI5ARROWS_14:
            {
                m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Arrows5;
                break;
            }
            case KPISets14::KPI5ARROWSGRAY_14:
            {
                m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Arrows5Gray;
                break;
            }
            case KPISets14::KPI5RATING_14:
            {
                m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Rating5;
                break;
            }
            case KPISets14::KPI5QUARTERS_14:
            {
                m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Quarters5;
                break;
            }
            case KPISets14::KPI3STARS_14:
            {
                m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Stars3;
                break;
            }
            case KPISets14::KPI3TRIANGLES_14:
            {
                m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Triangles3;
                break;
            }
            case KPISets14::KPI5BOXES_14:
            {
                m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Boxes5;
                break;
            }
        }
    }
}
void CColorScale::toXML2(NSStringUtils::CStringBuilder& writer, bool bExtendedWrite) const
{
	if (1 < m_arrValues.size() && 1 < m_arrColors.size()) // min 2 + 2
	{
        std::wstring sValue;
		writer.WriteString(L"<colorScale>");

        for ( size_t i = 0; i < m_arrValues.size(); ++i)//todooo - проверить можно ли не чередовать,а как есть записать
        {
			if ( m_arrValues[i].IsInit() )
            {
                m_arrValues[i]->toXML2(writer, bExtendedWrite);
            }
			//if ( (i < m_arrColors.size()) && (m_arrColors[i].IsInit()) )
			//{
			//	m_arrColors[i]->toXML(writer);
			//}
		}
        for ( size_t i = 0/*m_arrValues.size()*/; i < m_arrColors.size(); ++i)
        {
			if ( m_arrColors[i].IsInit() )
            {
                m_arrColors[i]->toXML(writer);
            }
		}
		writer.WriteString(L"</colorScale>");
	}
}
void CColorScale::fromXML(XmlUtils::CXmlLiteReader& oReader)
{
	if (oReader.IsEmptyNode())
		return;

	int nCurDepth = oReader.GetDepth();
	while (oReader.ReadNextSiblingNode(nCurDepth))
	{
		std::wstring sName = XmlUtils::GetNameNoNS(oReader.GetName());
		if (L"cfvo" == sName)
		{
			nullable<CConditionalFormatValueObject> item(oReader);
			m_arrValues.push_back(item);
		}
		else if (L"color" == sName)
		{
			nullable<CColor> item(oReader);
			m_arrColors.push_back(item);
		}
	}
}

CColorScale::CColorScale()
{
}
CColorScale::~CColorScale()
{
}
void CColorScale::fromXML(XmlUtils::CXmlNode& node)
{
}
std::wstring CColorScale::toXML() const
{
	return L"";
}
void CColorScale::toXML(NSStringUtils::CStringBuilder& writer) const
{
	toXML2(writer, false);
}
void CColorScale::fromBin(XLS::BaseObjectPtr& obj)
{
    if(obj->get_type() == XLS::typeCOLORSCALE)
    {
        auto ptr = static_cast<XLSB::COLORSCALE*>(obj.get());

        if(ptr != nullptr)
        {
            for(auto &cfvo : ptr->m_arCFVO)
            {
                nullable<CConditionalFormatValueObject> item(cfvo);
                m_arrValues.push_back(item);
            }
            for(auto &color : ptr->m_arBrtColor)
            {
                nullable<CColor> item(color);
                m_arrColors.push_back(item);
            }

        }
    }
    else if(obj->get_type() == XLS::typeCOLORSCALE14)
    {
        auto ptr = static_cast<XLSB::COLORSCALE14*>(obj.get());

        if(ptr != nullptr)
        {
            for(auto &cfvo14 : ptr->m_arCFVO14)
            {
                nullable<CConditionalFormatValueObject> item(cfvo14);
                m_arrValues.push_back(item);
            }
            for(auto &color14 : ptr->m_arBrtColor14)
            {
                auto color = static_cast<XLSB::Color14*>(color14.get());
                nullable<CColor> item;
                item.Init();
                item->fromBin(dynamic_cast<XLS::BaseObject*>(&color->color));
                m_arrColors.push_back(item);
            }

        }
    }
}
XLS::BaseObjectPtr CColorScale::toBin()
{
    auto ptr(new XLSB::COLORSCALE);
    XLS::BaseObjectPtr objectPtr(ptr);
    for(auto i:m_arrValues)
        ptr->m_arCFVO.push_back(i->toBin());
    for(auto i:m_arrColors)
        ptr->m_arBrtColor.push_back(i->toBin());
    return objectPtr;
}
void CColorScale::toBin(XLS::StreamCacheWriterPtr& writer)
{
    {
        auto begin = writer->getNextRecord(XLSB::rt_BeginColorScale);
        writer->storeNextRecord(begin);
    }
    for(auto i:m_arrValues)
        i->toBin(writer);
    for(auto i:m_arrColors)
        i->toBin(writer);
    {
        auto end = writer->getNextRecord(XLSB::rt_EndColorScale);
        writer->storeNextRecord(end);
    }
}
XLS::BaseObjectPtr CColorScale::toBin14()
{
    auto ptr(new XLSB::COLORSCALE14);
    XLS::BaseObjectPtr objectPtr(ptr);
    for(auto i:m_arrValues)
        ptr->m_arCFVO14.push_back(i->toBin14());
    for(auto i:m_arrColors)
        ptr->m_arBrtColor14.push_back(i->toBin14());
    return objectPtr;
}
EElementType CColorScale::getType () const
{
	return et_x_ColorScale;
}

template<typename Type>
nullable<Type> CColorScale::Merge(const nullable<Type> &oPrev, const nullable<Type> &oCurrent)
{
	nullable<Type> oResult;

	if ( oCurrent.IsInit() )	oResult = oCurrent;
	else if ( oPrev.IsInit() )	oResult = oPrev;

	return oResult;
}
const CColorScale CColorScale::Merge(const CColorScale& oPrev, const CColorScale& oCurrent)
{
	CColorScale oColorScale;

	for (size_t i = 0; i < oPrev.m_arrValues.size(); i++)
	{
		if (i < oCurrent.m_arrValues.size())
			oColorScale.m_arrValues.push_back(Merge( oPrev.m_arrValues[i],	oCurrent.m_arrValues[i] ));
		else
			oColorScale.m_arrValues.push_back(oPrev.m_arrValues[i]);
	}
	for (size_t i = oPrev.m_arrValues.size(); i < oCurrent.m_arrValues.size(); i++)
	{
		oColorScale.m_arrValues.push_back(oCurrent.m_arrValues[i]);
	}
	for (size_t i = 0; i < oPrev.m_arrColors.size(); i++)
	{
		if (i < oCurrent.m_arrColors.size())
			oColorScale.m_arrColors.push_back(Merge( oPrev.m_arrColors[i],	oCurrent.m_arrColors[i] ));
		else
			oColorScale.m_arrColors.push_back(oPrev.m_arrColors[i]);
	}
	for (size_t i = oPrev.m_arrColors.size(); i < oCurrent.m_arrColors.size(); i++)
	{
		oColorScale.m_arrColors.push_back(oCurrent.m_arrColors[i]);
	}
	return oColorScale;
}
bool CDataBar::isExtended ()
{
	if (m_oAxisColor.IsInit() || m_oAxisPosition.IsInit() || m_oDirection.IsInit() || m_oBorderColor.IsInit()
			|| m_oNegativeFillColor.IsInit() || m_oNegativeBorderColor.IsInit()
			|| m_oNegativeBarColorSameAsPositive.IsInit() || m_oNegativeBarBorderColorSameAsPositive.IsInit())
	{
		return true;
	}
	return false;
}

CDataBar::CDataBar()
{
}
CDataBar::~CDataBar()
{
}
void CDataBar::fromXML(XmlUtils::CXmlNode& node)
{
}
std::wstring CDataBar::toXML() const
{
	return L"";
}
void CDataBar::toXML(NSStringUtils::CStringBuilder& writer) const
{
	toXML2(writer, false);
}
EElementType CDataBar::getType () const
{
	return et_x_DataBar;
}
void CDataBar::toXML2(NSStringUtils::CStringBuilder& writer, bool bExtendedWrite) const
{
	if (2 != m_arrValues.size() || false == m_oColor.IsInit()) return;

	std::wstring node_name = bExtendedWrite ? L"x14:dataBar" : L"dataBar";

	writer.WriteString(L"<" + node_name);
		WritingStringNullableAttrInt(L"maxLength", m_oMaxLength, m_oMaxLength->GetValue());
		WritingStringNullableAttrInt(L"minLength", m_oMinLength, m_oMinLength->GetValue());
		if (m_oShowValue.IsInit() && false == m_oShowValue->ToBool())
		{
			writer.WriteString(L" showValue=\"0\"");
		}
		if (bExtendedWrite)
		{
			if (m_oBorderColor.IsInit()) writer.WriteString(L" border=\"1\"");
			WritingStringNullableAttrString(L"axisPosition", m_oAxisPosition, m_oAxisPosition->ToString())
			WritingStringNullableAttrString(L"direction", m_oDirection, m_oDirection->ToString())

			if (m_oGradient.IsInit() && false == m_oGradient->ToBool())
			{
				writer.WriteString(L" gradient=\"0\"");
			}
			if (m_oNegativeBarColorSameAsPositive.IsInit() && true == m_oNegativeBarColorSameAsPositive->ToBool())
			{
				writer.WriteString(L" negativeBarColorSameAsPositive=\"1\"");
			}
			if (m_oNegativeBarBorderColorSameAsPositive.IsInit() && false == m_oNegativeBarBorderColorSameAsPositive->ToBool())
			{
				writer.WriteString(L" negativeBarBorderColorSameAsPositive=\"0\"");
			}
		}
	writer.WriteString(L">");

    for ( size_t i = 0; i < m_arrValues.size(); ++i)
    {
        if (  m_arrValues[i].IsInit() )
        {
            m_arrValues[i]->toXML2(writer, bExtendedWrite);
        }
    }

	m_oColor->toXML2(writer, bExtendedWrite ? L"x14:fillColor" : L"color");

	if (bExtendedWrite)
	{
		if (m_oBorderColor.IsInit())		m_oBorderColor->toXML2(writer, L"x14:borderColor");
		if (m_oNegativeFillColor.IsInit())	m_oNegativeFillColor->toXML2(writer, L"x14:negativeFillColor");
		if (m_oNegativeBorderColor.IsInit())m_oNegativeBorderColor->toXML2(writer, L"x14:negativeBorderColor");
		if (m_oAxisColor.IsInit())			m_oAxisColor->toXML2(writer, L"x14:axisColor");
	}

	writer.WriteString(L"</" + node_name + L">");
}
void CDataBar::fromXML(XmlUtils::CXmlLiteReader& oReader)
{
	ReadAttributes(oReader);

	if (oReader.IsEmptyNode())
		return;

	int nCurDepth = oReader.GetDepth();
	while (oReader.ReadNextSiblingNode(nCurDepth))
	{
		std::wstring sName = XmlUtils::GetNameNoNS(oReader.GetName());
		if (L"cfvo" == sName)
		{
			nullable<CConditionalFormatValueObject> item = oReader;
			m_arrValues.push_back(item);
		}
		else if ( L"color" == sName || L"fillColor" == sName )
			m_oColor = oReader;
		else if ( L"axisColor" == sName)
			m_oAxisColor = oReader;
		else if ( L"borderColor" == sName)
			m_oBorderColor = oReader;
		else if ( L"negativeFillColor" == sName )
			m_oNegativeFillColor = oReader;
		else if ( L"negativeBorderColor" == sName )
			m_oNegativeBorderColor = oReader;
	}
}
void CDataBar::fromBin(XLS::BaseObjectPtr& obj)
{
    if(obj->get_type() == XLS::typeDATABAR)
    {
        auto ptr = static_cast<XLSB::DATABAR*>(obj.get());

        if(ptr != nullptr)
        {
            ReadAttributes(ptr->m_BrtBeginDatabar);

            for(auto &cfvo : ptr->m_arCFVO)
            {
                nullable<CConditionalFormatValueObject> item(cfvo);
                m_arrValues.push_back(item);
            }
            m_oColor = ptr->m_BrtColor;
        }
    }
    else if(obj->get_type() == XLS::typeDATABAR14)
    {
        auto ptr = static_cast<XLSB::DATABAR14*>(obj.get());

        if(ptr != nullptr)
        {
            ReadAttributes(ptr->m_BrtBeginDatabar14);

            for(auto &cfvo14 : ptr->m_arCFVO14)
            {
                nullable<CConditionalFormatValueObject> item(cfvo14);
                m_arrValues.push_back(item);
            }
            for(size_t i = 0; i < ptr->m_arBrtColor14.size(); ++i)
            {
                auto color = static_cast<XLSB::Color14*>(ptr->m_arBrtColor14[i].get());
                if(ptr->iPri != -1 && !m_oColor.IsInit())
                {
                    m_oColor.Init();
                    m_oColor->fromBin(dynamic_cast<XLS::BaseObject*>(&color->color));
                }
                else if(m_oBorder.IsInit() && m_oBorder->GetValue() == true && !m_oBorderColor.IsInit())
                {
                    m_oBorderColor.Init();
                    m_oBorderColor->fromBin(dynamic_cast<XLS::BaseObject*>(&color->color));
                }
                else if(m_oNegativeBarColorSameAsPositive.IsInit() && m_oNegativeBarColorSameAsPositive->GetValue() == false && !m_oNegativeFillColor.IsInit())
                {
                    m_oNegativeFillColor.Init();
                    m_oNegativeFillColor->fromBin(dynamic_cast<XLS::BaseObject*>(&color->color));
                }
                else if(m_oNegativeBarBorderColorSameAsPositive.IsInit() && m_oNegativeBarBorderColorSameAsPositive->GetValue() == false && !m_oNegativeBorderColor.IsInit())
                {
                    m_oNegativeBorderColor.Init();
                    m_oNegativeBorderColor->fromBin(dynamic_cast<XLS::BaseObject*>(&color->color));
                }
                else if(m_oAxisPosition.IsInit() && m_oAxisPosition->GetValue() != 2 && !m_oAxisColor.IsInit())
                {
                    m_oAxisColor.Init();
                    m_oAxisColor->fromBin(dynamic_cast<XLS::BaseObject*>(&color->color));
                }

            }
        }
    }

}
XLS::BaseObjectPtr CDataBar::toBin()
{
    auto ptr(new XLSB::DATABAR);
    XLS::BaseObjectPtr objectPtr(ptr);

    auto ptr1(new XLSB::BeginDatabar);
    ptr->m_BrtBeginDatabar = XLS::BaseObjectPtr{ptr1};
    if(m_oMaxLength.IsInit())
        ptr1->bLenMax = m_oMaxLength->GetValue();

    if(m_oMinLength.IsInit())
        ptr1->bLenMin = m_oMinLength->GetValue();

    if(m_oShowValue.IsInit())
        ptr1->fShowValue = m_oShowValue->GetValue();

    for(auto i:m_arrValues)
        ptr->m_arCFVO.push_back(i->toBin());
    if(m_oColor.IsInit())
        ptr->m_BrtColor = m_oColor->toBin();

    return objectPtr;
}
void CDataBar::toBin(XLS::StreamCacheWriterPtr& writer)
{
    {
        auto begin = writer->getNextRecord(XLSB::rt_BeginDatabar);
        BYTE bLenMin = 0;
        BYTE bLenMax = 100;
        BYTE fShowValue = true;
        if(m_oMaxLength.IsInit())
            bLenMax = m_oMaxLength->GetValue();
        if(m_oMinLength.IsInit())
            bLenMin = m_oMinLength->GetValue();
        if(m_oShowValue.IsInit())
            fShowValue = m_oShowValue->GetValue();
        *begin << bLenMin << bLenMax << fShowValue;
        writer->storeNextRecord(begin);
    }
    for(auto i:m_arrValues)
        i->toBin(writer);
    if(m_oColor.IsInit())
        m_oColor->toBin(writer);
    {
        auto end = writer->getNextRecord(XLSB::rt_EndDatabar);
        writer->storeNextRecord(end);
    }
}
XLS::BaseObjectPtr CDataBar::toBin14()
{
    auto ptr(new XLSB::DATABAR14);
    XLS::BaseObjectPtr objectPtr(ptr);

    auto ptr1(new XLSB::BeginDatabar14);
    ptr->m_BrtBeginDatabar14 = XLS::BaseObjectPtr{ptr1};
    if(m_oMaxLength.IsInit())
        ptr1->bLenMax = m_oMaxLength->GetValue();
    else
        ptr1->bLenMax = 100;
    if(m_oMinLength.IsInit())
        ptr1->bLenMin = m_oMinLength->GetValue();
    else
        ptr1->bLenMin = 0;
    if(m_oShowValue.IsInit())
        ptr1->fShowValue = m_oShowValue->GetValue();
    if(m_oGradient.IsInit())
        ptr1->fGradient = m_oGradient->GetValue();
    if(m_oDirection.IsInit())
        ptr1->bDirection = m_oDirection->GetValue();
    if(m_oAxisPosition.IsInit())
        ptr1->bAxisPosType = m_oAxisPosition->GetValue();
    if(m_oBorder.IsInit())
        ptr1->fBorder = m_oBorder->GetValue();
    if(m_oNegativeBarColorSameAsPositive.IsInit())
        ptr1->fCustomNegativeFillColor = !m_oNegativeBarColorSameAsPositive->GetValue();
    if(m_oNegativeBarBorderColorSameAsPositive.IsInit())
        ptr1->fCustomNegativeBorderColor = !m_oNegativeBarBorderColorSameAsPositive->GetValue();

    for(auto i:m_arrValues)
        ptr->m_arCFVO14.push_back(i->toBin14());
    if(m_oColor.IsInit())
        ptr->m_arBrtColor14.push_back(m_oColor->toBin14());
    if(m_oAxisColor.IsInit())
        ptr->m_arBrtColor14.push_back(m_oAxisColor->toBin14());
    if(m_oBorderColor.IsInit())
        ptr->m_arBrtColor14.push_back(m_oBorderColor->toBin14());
    if(m_oNegativeFillColor.IsInit())
        ptr->m_arBrtColor14.push_back(m_oNegativeFillColor->toBin14());
    if(m_oNegativeBorderColor.IsInit())
        ptr->m_arBrtColor14.push_back(m_oNegativeBorderColor->toBin14());

    return objectPtr;
}

void CDataBar::ReadAttributes(XLS::BaseObjectPtr& obj)
{
    if(obj->get_type() == XLS::typeBeginDatabar)
    {
        auto ptr = static_cast<XLSB::BeginDatabar*>(obj.get());

        if(ptr != nullptr)
        {
            m_oMaxLength = ptr->bLenMax;
            m_oMinLength = ptr->bLenMin;
            m_oShowValue = (bool)ptr->fShowValue;
        }
    }

    else if(obj->get_type() == XLS::typeBeginDatabar14)
    {
        auto ptr = static_cast<XLSB::BeginDatabar14*>(obj.get());

        if(ptr != nullptr)
        {
            m_oMaxLength = ptr->bLenMax;
            m_oMinLength = ptr->bLenMin;
            m_oShowValue = (bool)ptr->fShowValue;
            //ext
            m_oBorder                                  = ptr->fBorder;
            m_oGradient                                = ptr->fGradient;
            m_oNegativeBarColorSameAsPositive          = !ptr->fCustomNegativeFillColor;
            m_oNegativeBarBorderColorSameAsPositive    = !ptr->fCustomNegativeBorderColor;
            m_oAxisPosition                            = (SimpleTypes::Spreadsheet::EDataBarAxisPosition)ptr->bAxisPosType;
            m_oDirection                               = (SimpleTypes::Spreadsheet::EDataBarDirection)ptr->bDirection;
        }
    }

}

template<typename Type>
nullable<Type> CDataBar::Merge(const nullable<Type> &oPrev, const nullable<Type> &oCurrent)
{
	nullable<Type> oResult;

	if ( oCurrent.IsInit() )	oResult = oCurrent;
	else if ( oPrev.IsInit() )	oResult = oPrev;

	return oResult;
}
const CDataBar CDataBar::Merge(const CDataBar& oPrev, const CDataBar& oCurrent)
{
	CDataBar oDataBar;
	oDataBar.m_oMaxLength		= Merge( oPrev.m_oMaxLength,            oCurrent.m_oMaxLength );
	oDataBar.m_oMinLength		= Merge( oPrev.m_oMinLength,            oCurrent.m_oMinLength );
	oDataBar.m_oShowValue		= Merge( oPrev.m_oShowValue,            oCurrent.m_oShowValue );
	oDataBar.m_oAxisPosition	= Merge( oPrev.m_oAxisPosition,			oCurrent.m_oAxisPosition );
	oDataBar.m_oBorder			= Merge( oPrev.m_oBorder,				oCurrent.m_oBorder );
	oDataBar.m_oGradient		= Merge( oPrev.m_oGradient,				oCurrent.m_oGradient );
	oDataBar.m_oDirection		= Merge( oPrev.m_oDirection,			oCurrent.m_oDirection );
	oDataBar.m_oNegativeBarColorSameAsPositive = Merge( oPrev.m_oNegativeBarColorSameAsPositive, oCurrent.m_oNegativeBarColorSameAsPositive );
	oDataBar.m_oNegativeBarBorderColorSameAsPositive = Merge( oPrev.m_oNegativeBarBorderColorSameAsPositive, oCurrent.m_oNegativeBarBorderColorSameAsPositive );

	oDataBar.m_oColor				= Merge( oPrev.m_oColor,				oCurrent.m_oColor );
	oDataBar.m_oAxisColor			= Merge( oPrev.m_oAxisColor,			oCurrent.m_oAxisColor );
	oDataBar.m_oBorderColor			= Merge( oPrev.m_oBorderColor,			oCurrent.m_oBorderColor );
	oDataBar.m_oNegativeFillColor	= Merge( oPrev.m_oNegativeFillColor,	oCurrent.m_oNegativeFillColor );
	oDataBar.m_oNegativeBorderColor	= Merge( oPrev.m_oNegativeBorderColor,	oCurrent.m_oNegativeBorderColor );

	for (size_t i = 0; i < oPrev.m_arrValues.size(); i++)
	{
		if (i < oCurrent.m_arrValues.size())
			oDataBar.m_arrValues.push_back(Merge( oPrev.m_arrValues[i],	oCurrent.m_arrValues[i] ));
		else
			oDataBar.m_arrValues.push_back(oPrev.m_arrValues[i]);
	}
	for (size_t i = oPrev.m_arrValues.size(); i < oCurrent.m_arrValues.size(); i++)
	{
		oDataBar.m_arrValues.push_back(oCurrent.m_arrValues[i]);
	}
	return oDataBar;
}
void CDataBar::ReadAttributes(XmlUtils::CXmlLiteReader& oReader)
{
	WritingElement_ReadAttributes_Start(oReader)
		WritingElement_ReadAttributes_Read_if		(oReader, L"maxLength"	, m_oMaxLength)
		WritingElement_ReadAttributes_Read_else_if	(oReader, L"minLength"	, m_oMinLength)
		WritingElement_ReadAttributes_Read_else_if	(oReader, L"showValue"	, m_oShowValue)

		WritingElement_ReadAttributes_Read_else_if	(oReader, L"axisPosition", m_oAxisPosition)
		WritingElement_ReadAttributes_Read_else_if	(oReader, L"border"		, m_oBorder)
		WritingElement_ReadAttributes_Read_else_if	(oReader, L"gradient"	, m_oGradient)
		WritingElement_ReadAttributes_Read_else_if	(oReader, L"direction"	, m_oDirection)
		WritingElement_ReadAttributes_Read_else_if	(oReader, L"negativeBarColorSameAsPositive", m_oNegativeBarColorSameAsPositive)
		WritingElement_ReadAttributes_Read_else_if	(oReader, L"negativeBarBorderColorSameAsPositive", m_oNegativeBarBorderColorSameAsPositive)
	WritingElement_ReadAttributes_End(oReader)
}

CIconSet::CIconSet()
{
}
CIconSet::~CIconSet()
{
}
void CIconSet::fromXML(XmlUtils::CXmlNode& node)
{
}
std::wstring CIconSet::toXML() const
{
	return L"";
}
void CIconSet::toXML(NSStringUtils::CStringBuilder& writer) const
{
	toXML2(writer, false);
}
EElementType CIconSet::getType () const
{
	return et_x_IconSet;
}
void CIconSet::toXML2(NSStringUtils::CStringBuilder& writer, bool bExtendedWrite) const
{
	if (m_arrValues.size() < 2) return;	// min value = 2

	std::wstring node_name = bExtendedWrite ? L"x14:iconSet" : L"iconSet";

	std::wstring sValue;
	writer.WriteString(L"<" + node_name);
	WritingStringNullableAttrString(L"iconSet", m_oIconSet, m_oIconSet->ToString())
	if (m_oPercent.IsInit() && false == m_oPercent->ToBool())
	{
		writer.WriteString(L" percent=\"0\"");
	}
	if (m_oReverse.IsInit() && true == m_oReverse->ToBool())
	{
		writer.WriteString(L" reverse=\"1\"");
	}
	if (m_oShowValue.IsInit() && false == m_oShowValue->ToBool())
	{
		writer.WriteString(L" showValue=\"0\"");
	}
	if (bExtendedWrite && false == m_arrIconSets.empty())
	{
		writer.WriteString(L" custom=\"1\"");
	}
	writer.WriteString(L">");

    for ( size_t i = 0; i < m_arrValues.size(); ++i)
    {
		if ( m_arrValues[i].IsInit() )
        {
            m_arrValues[i]->toXML2(writer, bExtendedWrite);
        }
	}
    for ( size_t i = 0; bExtendedWrite && i < m_arrIconSets.size(); ++i)
    {
		if ( m_arrIconSets[i].IsInit() )
        {
            m_arrIconSets[i]->toXML2(writer, bExtendedWrite);
        }
	}
	writer.WriteString(L"</" + node_name + L">");
}
void CIconSet::fromXML(XmlUtils::CXmlLiteReader& oReader)
{
	ReadAttributes(oReader);

	if (oReader.IsEmptyNode())
		return;

	int nCurDepth = oReader.GetDepth();
	while (oReader.ReadNextSiblingNode(nCurDepth))
	{
		std::wstring sName = XmlUtils::GetNameNoNS(oReader.GetName());
		if (L"cfvo" == sName)
		{
			nullable<CConditionalFormatValueObject> item(oReader);
			m_arrValues.push_back(item);
		}
		else if (L"cfIcon" == sName)
		{
			nullable<CConditionalFormatIconSet> item(oReader);
			m_arrIconSets.push_back(item);
		}
	}
}

void CIconSet::fromBin(XLS::BaseObjectPtr& obj)
{
    if(obj->get_type() == XLS::typeICONSET)
    {
        auto ptr = static_cast<XLSB::ICONSET*>(obj.get());

        if(ptr != nullptr)
        {
            ReadAttributes(ptr->m_BrtBeginIconSet);

            for(auto &cfvo : ptr->m_arCFVO)
            {
                nullable<CConditionalFormatValueObject> item(cfvo);
                m_arrValues.push_back(item);
            }
        }
    }
    else if(obj->get_type() == XLS::typeICONSET14)
    {
        auto ptr = static_cast<XLSB::ICONSET14*>(obj.get());

        if(ptr != nullptr)
        {
            ReadAttributes(ptr->m_BrtBeginIconSet14);

            for(auto &cfvo14 : ptr->m_arCFVO14)
            {
                nullable<CConditionalFormatValueObject> item(cfvo14);
                m_arrValues.push_back(item);
            }

            for(auto &cficon : ptr->m_arBrtCFIcon)
            {
                nullable<CConditionalFormatIconSet> item(cficon);
                m_arrIconSets.push_back(item);
            }
        }
    }
}
XLS::BaseObjectPtr CIconSet::toBin()
{
    auto ptr(new XLSB::ICONSET);
    XLS::BaseObjectPtr objectPtr(ptr);

    auto beginPtr(new XLSB::BeginIconSet);
    ptr->m_BrtBeginIconSet = XLS::BaseObjectPtr{beginPtr};
    if(m_oShowValue.IsInit())
        beginPtr->fIcon = !m_oShowValue->GetValue();
    else
        beginPtr->fIcon = false;
    if(m_oReverse.IsInit())
        beginPtr->fReverse = m_oReverse->GetValue();
    else
        beginPtr->fReverse = false;
    if(m_oIconSet.IsInit())
    {
        switch (m_oIconSet->GetValue())
        {
            case SimpleTypes::Spreadsheet::EIconSetType::NoIcons:
            {
                beginPtr->iSet.set = KPISets::KPINIL;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Arrows3:
            {
                beginPtr->iSet.set = KPISets::KPI3ARROWS;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Arrows3Gray:
            {
                beginPtr->iSet.set = KPISets::KPI3ARROWSGRAY;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Flags3:
            {
                beginPtr->iSet.set = KPISets::KPI3FLAGS;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Traffic3Lights1:
            {
                beginPtr->iSet.set = KPISets::KPI3TRAFFICLIGHTS1;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Traffic3Lights2:
            {
                beginPtr->iSet.set = KPISets::KPI3TRAFFICLIGHTS2;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Signs3:
            {
                beginPtr->iSet.set = KPISets::KPI3SIGNS;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Symbols3:
            {
                beginPtr->iSet.set = KPISets::KPI3SYMBOLS;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Symbols3_2:
            {
                beginPtr->iSet.set = KPISets::KPI3SYMBOLS2;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Arrows4:
            {
                beginPtr->iSet.set = KPISets::KPI4ARROWS;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Arrows4Gray:
            {
                beginPtr->iSet.set = KPISets::KPI4ARROWSGRAY;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::RedToBlack4:
            {
                beginPtr->iSet.set = KPISets::KPI4REDTOBLACK;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Rating4:
            {
                beginPtr->iSet.set = KPISets::KPI4RATING;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Traffic4Lights:
            {
                beginPtr->iSet.set =  KPISets::KPI4TRAFFICLIGHTS;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Arrows5:
            {
                beginPtr->iSet.set = KPISets::KPI5ARROWS;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Arrows5Gray:
            {
                beginPtr->iSet.set = KPISets::KPI5ARROWSGRAY;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Rating5:
            {
                beginPtr->iSet.set = KPISets::KPI5RATING;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Quarters5:
            {
                beginPtr->iSet.set = KPISets::KPI5QUARTERS;
                break;
            }
            default:
            {
                beginPtr->iSet.set = KPISets::KPINIL;
                break;
            }
        }
    }
    else
    {
        switch(m_arrValues.size())
        {
            case 0:
            {
                beginPtr->iSet.set = KPISets::KPINIL;
                break;
            }
            case 3:
            {
                beginPtr->iSet.set = KPISets::KPI3TRAFFICLIGHTS1;
                break;
            }
            case 4:
            {
                beginPtr->iSet.set = KPISets::KPI4TRAFFICLIGHTS;
                break;
            }
            case 5:
            {
                beginPtr->iSet.set = KPISets::KPI5QUARTERS;
                break;
            }
            default:
            {
                beginPtr->iSet.set = KPISets::KPINIL;
                break;
            }
        }

    }

    for(auto i:m_arrValues)
        ptr->m_arCFVO.push_back(i->toBin(true));
    return objectPtr;
}
void CIconSet::toBin(XLS::StreamCacheWriterPtr& writer)
{
    {
        auto begin = writer->getNextRecord(XLSB::rt_BeginIconSet);
        _UINT32 iSet = 0;
        if(m_oIconSet.IsInit())
        {
            switch (m_oIconSet->GetValue())
            {
                case SimpleTypes::Spreadsheet::EIconSetType::NoIcons:
                {
                    iSet = KPISets::KPINIL;
                    break;
                }
                case SimpleTypes::Spreadsheet::EIconSetType::Arrows3:
                {
                    iSet = KPISets::KPI3ARROWS;
                    break;
                }
                case SimpleTypes::Spreadsheet::EIconSetType::Arrows3Gray:
                {
                    iSet = KPISets::KPI3ARROWSGRAY;
                    break;
                }
                case SimpleTypes::Spreadsheet::EIconSetType::Flags3:
                {
                    iSet = KPISets::KPI3FLAGS;
                    break;
                }
                case SimpleTypes::Spreadsheet::EIconSetType::Traffic3Lights1:
                {
                    iSet = KPISets::KPI3TRAFFICLIGHTS1;
                    break;
                }
                case SimpleTypes::Spreadsheet::EIconSetType::Traffic3Lights2:
                {
                    iSet = KPISets::KPI3TRAFFICLIGHTS2;
                    break;
                }
                case SimpleTypes::Spreadsheet::EIconSetType::Signs3:
                {
                    iSet = KPISets::KPI3SIGNS;
                    break;
                }
                case SimpleTypes::Spreadsheet::EIconSetType::Symbols3:
                {
                    iSet = KPISets::KPI3SYMBOLS;
                    break;
                }
                case SimpleTypes::Spreadsheet::EIconSetType::Symbols3_2:
                {
                    iSet = KPISets::KPI3SYMBOLS2;
                    break;
                }
                case SimpleTypes::Spreadsheet::EIconSetType::Arrows4:
                {
                    iSet = KPISets::KPI4ARROWS;
                    break;
                }
                case SimpleTypes::Spreadsheet::EIconSetType::Arrows4Gray:
                {
                    iSet = KPISets::KPI4ARROWSGRAY;
                    break;
                }
                case SimpleTypes::Spreadsheet::EIconSetType::RedToBlack4:
                {
                    iSet = KPISets::KPI4REDTOBLACK;
                    break;
                }
                case SimpleTypes::Spreadsheet::EIconSetType::Rating4:
                {
                    iSet = KPISets::KPI4RATING;
                    break;
                }
                case SimpleTypes::Spreadsheet::EIconSetType::Traffic4Lights:
                {
                    iSet =  KPISets::KPI4TRAFFICLIGHTS;
                    break;
                }
                case SimpleTypes::Spreadsheet::EIconSetType::Arrows5:
                {
                    iSet = KPISets::KPI5ARROWS;
                    break;
                }
                case SimpleTypes::Spreadsheet::EIconSetType::Arrows5Gray:
                {
                    iSet = KPISets::KPI5ARROWSGRAY;
                    break;
                }
                case SimpleTypes::Spreadsheet::EIconSetType::Rating5:
                {
                    iSet = KPISets::KPI5RATING;
                    break;
                }
                case SimpleTypes::Spreadsheet::EIconSetType::Quarters5:
                {
                    iSet = KPISets::KPI5QUARTERS;
                    break;
                }
                default:
                {
                    iSet = KPISets::KPINIL;
                    break;
                }
            }
        }
        else
        {
            switch(m_arrValues.size())
            {
                case 0:
                {
                    iSet = KPISets::KPINIL;
                    break;
                }
                case 3:
                {
                    iSet = KPISets::KPI3TRAFFICLIGHTS1;
                    break;
                }
                case 4:
                {
                    iSet = KPISets::KPI4TRAFFICLIGHTS;
                    break;
                }
                case 5:
                {
                    iSet = KPISets::KPI5QUARTERS;
                    break;
                }
                default:
                {
                    iSet = KPISets::KPINIL;
                    break;
                }
            }

        }
        _UINT16 flags = 0;
        if(m_oShowValue.IsInit())
            SETBIT(flags, 1, m_oShowValue->GetValue())
        if(m_oReverse.IsInit())
            SETBIT(flags, 2, m_oReverse->GetValue())
        *begin << iSet << flags;
        writer->storeNextRecord(begin);
    }
    for(auto i:m_arrValues)
        i->toBin(writer, true);
    {
        auto end = writer->getNextRecord(XLSB::rt_EndIconSet);
        writer->storeNextRecord(end);
    }
}
XLS::BaseObjectPtr CIconSet::toBin14()
{
    auto ptr(new XLSB::ICONSET14);
    XLS::BaseObjectPtr objectPtr(ptr);

    auto beginPtr(new XLSB::BeginIconSet14);
    ptr->m_BrtBeginIconSet14 = XLS::BaseObjectPtr{beginPtr};
    if(m_oShowValue.IsInit())
        beginPtr->fIcon = !m_oShowValue->GetValue();
    if(m_oReverse.IsInit())
        beginPtr->fReverse = m_oReverse->GetValue();
    if(m_oIconSet.IsInit())
    {
        switch (m_oIconSet->GetValue())
        {
            case SimpleTypes::Spreadsheet::EIconSetType::NoIcons:
            {
                beginPtr->iSet.set = KPISets14::KPINIL_14;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Arrows3:
            {
                beginPtr->iSet.set = KPISets14::KPI3ARROWS_14;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Arrows3Gray:
            {
                beginPtr->iSet.set = KPISets14::KPI3ARROWSGRAY_14;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Flags3:
            {
                beginPtr->iSet.set = KPISets14::KPI3FLAGS_14;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Traffic3Lights1:
            {
                beginPtr->iSet.set = KPISets14::KPI3TRAFFICLIGHTS1_14;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Traffic3Lights2:
            {
                beginPtr->iSet.set = KPISets14::KPI3TRAFFICLIGHTS2_14;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Signs3:
            {
                beginPtr->iSet.set = KPISets14::KPI3SIGNS_14;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Symbols3:
            {
                beginPtr->iSet.set = KPISets14::KPI3SYMBOLS_14;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Symbols3_2:
            {
                beginPtr->iSet.set = KPISets14::KPI3SYMBOLS2_14;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Arrows4:
            {
                beginPtr->iSet.set = KPISets14::KPI4ARROWS_14;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Arrows4Gray:
            {
                beginPtr->iSet.set = KPISets14::KPI4ARROWSGRAY_14;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::RedToBlack4:
            {
                beginPtr->iSet.set = KPISets14::KPI4REDTOBLACK_14;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Rating4:
            {
                beginPtr->iSet.set = KPISets14::KPI4RATING_14;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Traffic4Lights:
            {
                beginPtr->iSet.set =  KPISets14::KPI4TRAFFICLIGHTS_14;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Arrows5:
            {
                beginPtr->iSet.set = KPISets14::KPI5ARROWS_14;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Arrows5Gray:
            {
                beginPtr->iSet.set = KPISets14::KPI5ARROWSGRAY_14;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Rating5:
            {
                beginPtr->iSet.set = KPISets14::KPI5RATING_14;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Quarters5:
            {
                beginPtr->iSet.set = KPISets14::KPI5QUARTERS_14;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Stars3:
            {
                beginPtr->iSet.set = KPISets14::KPI3STARS_14;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Triangles3:
            {
                beginPtr->iSet.set = KPISets14::KPI3TRIANGLES_14;
                break;
            }
            case SimpleTypes::Spreadsheet::EIconSetType::Boxes5:
            {
                beginPtr->iSet.set = KPISets14::KPI5BOXES_14;
                break;
            }
            default:
            {
                beginPtr->iSet.set = KPISets14::KPINIL_14;
                break;
            }
        }
    }
    else
    {
        switch(m_arrValues.size())
        {
            case 0:
            {
                beginPtr->iSet.set = KPISets14::KPINIL_14;
                break;
            }
            case 3:
            {
                beginPtr->iSet.set = KPISets14::KPI3TRAFFICLIGHTS1_14;
                break;
            }
            case 4:
            {
                beginPtr->iSet.set = KPISets14::KPI4TRAFFICLIGHTS_14;
                break;
            }
            case 5:
            {
                beginPtr->iSet.set = KPISets14::KPI5QUARTERS_14;
                break;
            }
            default:
            {
                beginPtr->iSet.set = KPISets14::KPINIL_14;
                break;
            }
        }

    }

    for(auto i:m_arrValues)
        ptr->m_arCFVO14.push_back(i->toBin14(true));
    return objectPtr;
}
void CIconSet::ReadAttributes(XLS::BaseObjectPtr& obj)
{
    if(obj->get_type() == XLS::typeBeginIconSet)
    {
        auto ptr = static_cast<XLSB::BeginIconSet*>(obj.get());

        if(ptr != nullptr)
        {
            m_oShowValue = !ptr->fIcon;
            m_oReverse   = ptr->fReverse;

            switch(ptr->iSet.set)
            {
                case KPISets::KPINIL:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::NoIcons;
                    break;
                }
                case KPISets::KPI3ARROWS:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Arrows3;
                    break;
                }
                case KPISets::KPI3ARROWSGRAY:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Arrows3Gray;
                    break;
                }
                case KPISets::KPI3FLAGS:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Flags3;
                    break;
                }
                case KPISets::KPI3TRAFFICLIGHTS1:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Traffic3Lights1;
                    break;
                }
                case KPISets::KPI3TRAFFICLIGHTS2:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Traffic3Lights2;
                    break;
                }
                case KPISets::KPI3SIGNS:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Signs3;
                    break;
                }
                case KPISets::KPI3SYMBOLS:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Symbols3;
                    break;
                }
                case KPISets::KPI3SYMBOLS2:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Symbols3_2;
                    break;
                }
                case KPISets::KPI4ARROWS:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Arrows4;
                    break;
                }
                case KPISets::KPI4ARROWSGRAY:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Arrows4Gray;
                    break;
                }
                case KPISets::KPI4REDTOBLACK:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::RedToBlack4;
                    break;
                }
                case KPISets::KPI4RATING:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Rating4;
                    break;
                }
                case KPISets::KPI4TRAFFICLIGHTS:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Traffic4Lights;
                    break;
                }
                case KPISets::KPI5ARROWS:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Arrows5;
                    break;
                }
                case KPISets::KPI5ARROWSGRAY:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Arrows5Gray;
                    break;
                }
                case KPISets::KPI5RATING:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Rating5;
                    break;
                }
                case KPISets::KPI5QUARTERS:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Quarters5;
                    break;
                }
            }

        }
    }
    else if(obj->get_type() == XLS::typeBeginIconSet14)
    {
        auto ptr = static_cast<XLSB::BeginIconSet14*>(obj.get());

        if(ptr != nullptr)
        {
            m_oShowValue = !ptr->fIcon;
            m_oReverse   = ptr->fReverse;
            m_oCustom    = ptr->fCustom;

            switch(ptr->iSet.set)
            {
                case KPISets14::KPINIL_14:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::NoIcons;
                    break;
                }
                case KPISets14::KPI3ARROWS_14:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Arrows3;
                    break;
                }
                case KPISets14::KPI3ARROWSGRAY_14:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Arrows3Gray;
                    break;
                }
                case KPISets14::KPI3FLAGS_14:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Flags3;
                    break;
                }
                case KPISets14::KPI3TRAFFICLIGHTS1_14:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Traffic3Lights1;
                    break;
                }
                case KPISets14::KPI3TRAFFICLIGHTS2_14:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Traffic3Lights2;
                    break;
                }
                case KPISets14::KPI3SIGNS_14:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Signs3;
                    break;
                }
                case KPISets14::KPI3SYMBOLS_14:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Symbols3;
                    break;
                }
                case KPISets14::KPI3SYMBOLS2_14:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Symbols3_2;
                    break;
                }
                case KPISets14::KPI4ARROWS_14:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Arrows4;
                    break;
                }
                case KPISets14::KPI4ARROWSGRAY_14:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Arrows4Gray;
                    break;
                }
                case KPISets14::KPI4REDTOBLACK_14:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::RedToBlack4;
                    break;
                }
                case KPISets14::KPI4RATING_14:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Rating4;
                    break;
                }
                case KPISets14::KPI4TRAFFICLIGHTS_14:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Traffic4Lights;
                    break;
                }
                case KPISets14::KPI5ARROWS_14:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Arrows5;
                    break;
                }
                case KPISets14::KPI5ARROWSGRAY_14:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Arrows5Gray;
                    break;
                }
                case KPISets14::KPI5RATING_14:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Rating5;
                    break;
                }
                case KPISets14::KPI5QUARTERS_14:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Quarters5;
                    break;
                }
                case KPISets14::KPI3STARS_14:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Stars3;
                    break;
                }
                case KPISets14::KPI3TRIANGLES_14:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Triangles3;
                    break;
                }
                case KPISets14::KPI5BOXES_14:
                {
                    m_oIconSet = SimpleTypes::Spreadsheet::EIconSetType::Boxes5;
                    break;
                }
            }

        }
    }
}

template<typename Type>
nullable<Type> CIconSet::Merge(const nullable<Type> &oPrev, const nullable<Type> &oCurrent)
{
	nullable<Type> oResult;

	if ( oCurrent.IsInit() )	oResult = oCurrent;
	else if ( oPrev.IsInit() )	oResult = oPrev;

	return oResult;
}
const CIconSet CIconSet::Merge(const CIconSet& oPrev, const CIconSet& oCurrent)
{
	CIconSet oIconSet;
	oIconSet.m_oIconSet		= Merge( oPrev.m_oIconSet,		oCurrent.m_oIconSet );
	oIconSet.m_oPercent		= Merge( oPrev.m_oPercent,		oCurrent.m_oPercent );
	oIconSet.m_oReverse		= Merge( oPrev.m_oReverse,		oCurrent.m_oReverse );
	oIconSet.m_oShowValue	= Merge( oPrev.m_oShowValue,	oCurrent.m_oShowValue );
	oIconSet.m_oCustom		= Merge( oPrev.m_oCustom,		oCurrent.m_oCustom );

	for (size_t i = 0; i < oPrev.m_arrValues.size(); i++)
	{
		if (i < oCurrent.m_arrValues.size())
			oIconSet.m_arrValues.push_back(Merge( oPrev.m_arrValues[i],	oCurrent.m_arrValues[i] ));
		else
			oIconSet.m_arrValues.push_back(oPrev.m_arrValues[i]);
	}
	for (size_t i = oPrev.m_arrValues.size(); i < oCurrent.m_arrValues.size(); i++)
	{
		oIconSet.m_arrValues.push_back(oCurrent.m_arrValues[i]);
	}
	for (size_t i = 0; i < oPrev.m_arrIconSets.size(); i++)
	{
		if (i < oCurrent.m_arrIconSets.size())
			oIconSet.m_arrIconSets.push_back(Merge( oPrev.m_arrIconSets[i],	oCurrent.m_arrIconSets[i] ));
		else
			oIconSet.m_arrIconSets.push_back(oPrev.m_arrIconSets[i]);
	}
	for (size_t i = oPrev.m_arrIconSets.size(); i < oCurrent.m_arrIconSets.size(); i++)
	{
		oIconSet.m_arrIconSets.push_back(oCurrent.m_arrIconSets[i]);
	}
	return oIconSet;
}
bool CIconSet::isExtended ()
{
	return (false == m_arrIconSets.empty() || (m_oIconSet.IsInit() && m_oIconSet->GetValue() > 15));
}
void CIconSet::ReadAttributes(XmlUtils::CXmlLiteReader& oReader)
{
	WritingElement_ReadAttributes_Start(oReader)

	WritingElement_ReadAttributes_Read_if		(oReader, L"iconSet"	, m_oIconSet)
	WritingElement_ReadAttributes_Read_else_if	(oReader, L"percent"	, m_oPercent)
	WritingElement_ReadAttributes_Read_else_if	(oReader, L"reverse"	, m_oReverse)
	WritingElement_ReadAttributes_Read_else_if	(oReader, L"showValue"	, m_oShowValue)
	WritingElement_ReadAttributes_Read_else_if	(oReader, L"custom"		, m_oCustom)

	WritingElement_ReadAttributes_End(oReader)
}

CConditionalFormattingRule::CConditionalFormattingRule() : bUsage (false)
{
}
CConditionalFormattingRule::~CConditionalFormattingRule()
{
}
void CConditionalFormattingRule::fromXML(XmlUtils::CXmlNode& node)
{
}
std::wstring CConditionalFormattingRule::toXML() const
{
	return L"";
}
void CConditionalFormattingRule::toXML(NSStringUtils::CStringBuilder& writer) const
{
	toXML2(writer, false);
}
EElementType CConditionalFormattingRule::getType () const
{
	return et_x_ConditionalFormattingRule;
}
bool CConditionalFormattingRule::isValid () const
{
	return m_oType.IsInit() && m_oPriority.IsInit();
}
bool CConditionalFormattingRule::isExtended()
{
	if (m_oDxf.IsInit())		return true;

	if (m_oDataBar.IsInit())	return m_oDataBar->isExtended();
	if (m_oIconSet.IsInit())	return m_oIconSet->isExtended();
	if (m_oColorScale.IsInit())	return m_oColorScale->isExtended();

	for (size_t i = 0; i < m_arrFormula.size(); i++)
	{
		if ((m_arrFormula[i].IsInit()) && m_arrFormula[i]->isExtended())
			return true;
	}
	return false;
}
void CConditionalFormattingRule::toXML2(NSStringUtils::CStringBuilder& writer, bool bExtendedWrite) const
{
	if (false == isValid()) return;

	std::wstring node_name = bExtendedWrite ? L"x14:cfRule" : L"cfRule";

	writer.WriteString(L"<" + node_name);
		WritingStringNullableAttrString(L"type", m_oType, m_oType->ToString());
		WritingStringNullableAttrInt(L"priority", m_oPriority, m_oPriority->GetValue());
		if (m_oAboveAverage.IsInit() && false == m_oAboveAverage->ToBool())
			writer.WriteString(L" aboveAverage=\"0\"");
		if (m_oBottom.IsInit() && true == m_oBottom->ToBool())
			writer.WriteString(L" bottom=\"1\"");
		WritingStringNullableAttrInt(L"dxfId", m_oDxfId, m_oDxfId->GetValue());
		if (m_oEqualAverage.IsInit() && true == m_oEqualAverage->ToBool())
			writer.WriteString(L" equalAverage=\"1\"");
		WritingStringNullableAttrString(L"operator", m_oOperator, m_oOperator->ToString());
		if (m_oPercent.IsInit() && true == m_oPercent->ToBool())
			writer.WriteString(L" percent=\"1\"");
		WritingStringNullableAttrInt(L"rank", m_oRank, m_oRank->GetValue());
		WritingStringNullableAttrInt(L"stdDev", m_oStdDev, m_oStdDev->GetValue());
		if (m_oStopIfTrue.IsInit() && true == m_oStopIfTrue->ToBool())
			writer.WriteString(L" stopIfTrue=\"1\"");
		WritingStringNullableAttrEncodeXmlString(L"text", m_oText, m_oText.get());
		WritingStringNullableAttrString(L"timePeriod", m_oTimePeriod, m_oTimePeriod->ToString());

		if (bExtendedWrite)
		{
			if (false == m_oId.IsInit())
			{
				WritingStringAttrString(L"id", L"{" + XmlUtils::GenerateGuid() + L"}");
			}
			else
				WritingStringNullableAttrString(L"id", m_oId, m_oId.get());
		}
	writer.WriteString(L">");

	if (m_oIconSet.IsInit())
		m_oIconSet->toXML2(writer, bExtendedWrite);
	if (m_oColorScale.IsInit())
		m_oColorScale->toXML2(writer, bExtendedWrite);
	if (m_oDataBar.IsInit())
		m_oDataBar->toXML2(writer, bExtendedWrite);

	for (size_t i = 0; i < m_arrFormula.size(); i++)
	{
		if (m_arrFormula[i].IsInit())
			m_arrFormula[i]->toXML2(writer, bExtendedWrite);
	}

	if (m_oDxf.IsInit() && bExtendedWrite)
	{
		m_oDxf->toXML2(writer, L"x14:dxf");
	}

	writer.WriteString(L"</" + node_name + L">");
}
void CConditionalFormattingRule::fromXML(XmlUtils::CXmlLiteReader& oReader)
{
	bUsage = false;
	ReadAttributes(oReader);

	if (oReader.IsEmptyNode())
		return;

	int nCurDepth = oReader.GetDepth();
	while (oReader.ReadNextSiblingNode(nCurDepth))
	{
		std::wstring sName = XmlUtils::GetNameNoNS(oReader.GetName());

		if (L"colorScale" == sName )
			m_oColorScale = oReader;
		else if ( L"dataBar" == sName)
			m_oDataBar = oReader;
		else if ( L"formula" == sName || L"f" == sName)
		{
			nullable<CFormulaCF> formula(oReader);
			m_arrFormula.push_back(formula);
		}
		else if ( L"iconSet" == sName)
			m_oIconSet = oReader;
		else if ( L"dxf" == sName)
			m_oDxf = oReader;
		else if ( L"extLst" == sName )
		{
			m_oExtLst = oReader;
			if ( m_oExtLst.IsInit() )
			{
				for (size_t i = 0; i < m_oExtLst->m_arrExt.size(); ++i)
				{
					if (!m_oExtLst->m_arrExt[i]) continue;

					if (m_oExtLst->m_arrExt[i]->m_oId.IsInit())
					{
						m_oExtId = m_oExtLst->m_arrExt[i]->m_oId;
						break;
					}
				}
			}
		}

	}
}
void CConditionalFormattingRule::fromBin(XLS::BaseObjectPtr& obj)
{
    bUsage = false;
    if(obj->get_type() == XLS::typeCFRULE)
    {
        auto ptr = static_cast<XLSB::CFRULE*>(obj.get());

        ReadAttributes(ptr->m_BrtBeginCFRule);

        if(ptr->m_source != nullptr)
        {
            switch(ptr->m_source->get_type())
            {
                case typeCOLORSCALE:
                {
                    m_oColorScale = ptr->m_source;
                    break;
                }
                case typeDATABAR:
                {
                    m_oDataBar = ptr->m_source;
                    break;
                }
                case typeICONSET:
                {
                    m_oIconSet = ptr->m_source;
                    break;
                }
            }
        }
        if(ptr->m_FRTCFRULE != nullptr)
        {
            auto ptrFRTCFRULE = static_cast<XLSB::FRTCFRULE*>(ptr->m_FRTCFRULE.get());
            m_oExtId = static_cast<XLSB::CFRuleExt*>(ptrFRTCFRULE->m_BrtCFRuleExt.get())->guid;
        }
    }
    else if(obj->get_type() == XLS::typeCFRULE14)
    {
        auto ptr = static_cast<XLSB::CFRULE14*>(obj.get());

        ReadAttributes(ptr->m_BrtBeginCFRule14);

        if(ptr->m_source != nullptr)
        {
            switch(ptr->m_source->get_type())
            {
                case typeCOLORSCALE14:
                {
                    m_oColorScale = ptr->m_source;
                    break;
                }
                case typeDATABAR14:
                {
                    m_oDataBar = ptr->m_source;
                    break;
                }
                case typeICONSET14:
                {
                    m_oIconSet = ptr->m_source;
                    break;
                }
            }
        }
    }
}

XLS::BaseObjectPtr CConditionalFormattingRule::toBin(const  XLS::CellRef &cellRef)
{
    auto ptr(new XLSB::CFRULE(cellRef));
    XLS::BaseObjectPtr objPtr(ptr);

    ptr->m_BrtBeginCFRule = WriteAttributes(cellRef);

    if(m_oColorScale.IsInit())
    {
        ptr->m_source = m_oColorScale->toBin();
    }
    else if(m_oDataBar.IsInit())
    {
        ptr->m_source = m_oDataBar->toBin();
    }
    else if(m_oIconSet.IsInit())
    {
        ptr->m_source = m_oIconSet->toBin();
    }
    if(m_oExtId.IsInit())
    {
        auto extPtr(new XLSB::FRTCFRULE);
        auto beginExt(new XLSB::CFRuleExt);
        extPtr->m_BrtCFRuleExt = XLS::BaseObjectPtr{beginExt};
        ptr->m_FRTCFRULE = XLS::BaseObjectPtr{extPtr};

        beginExt->guid = m_oExtId.get();
        auto ruleBegin(new XLSB::FRTBegin);
        ruleBegin->productVersion.product = 0;
        ruleBegin->productVersion.version = 0;
        extPtr->m_BrtFRTBegin = XLS::BaseObjectPtr{ruleBegin};
    }
    return objPtr;
}

void CConditionalFormattingRule::toBin(XLS::StreamCacheWriterPtr& writer, const  XLS::CellRef &cellRef)
{
    {
        auto begin = writer->getNextRecord(XLSB::rt_BeginCFRule);
        WriteAttributes(begin, cellRef);
        writer->storeNextRecord(begin);
    }
    if(m_oColorScale.IsInit())
    {
        m_oColorScale->toBin(writer);
    }
    else if(m_oDataBar.IsInit())
    {
        m_oDataBar->toBin(writer);
    }
    else if(m_oIconSet.IsInit())
    {
        m_oIconSet->toBin(writer);
    }
    if(m_oExtId.IsInit())
    {
        XLSB::FRTCFRULE ext;
        auto beginExt(new XLSB::CFRuleExt);
        ext.m_BrtCFRuleExt = XLS::BaseObjectPtr{beginExt};

        beginExt->guid = m_oExtId.get();
        auto ruleBegin(new XLSB::FRTBegin);
        ext.m_BrtFRTBegin = XLS::BaseObjectPtr{ruleBegin};
        ext.write(writer, nullptr);
    }
    {
        auto end = writer->getNextRecord(XLSB::rt_EndCFRule);
        writer->storeNextRecord(end);
    }
}

XLS::BaseObjectPtr CConditionalFormattingRule::WriteAttributes(const  XLS::CellRef &cellRef)
{
    
    auto ptr(new XLSB::BeginCFRule(cellRef));
    BaseObjectPtr objectPtr(ptr);

    if(m_oDxfId.IsInit())
    {
        ptr->dxfId = m_oDxfId->GetValue();
    }
    else
        ptr->dxfId = 0;
    if(m_oPriority.IsInit())
        ptr->iPri = m_oPriority->GetValue();
    else
        ptr->iPri = 1;
    if(m_oStopIfTrue.IsInit())
        ptr->fStopTrue = m_oStopIfTrue->GetValue();
    else
        ptr->fStopTrue = false;
    if(m_oAboveAverage.IsInit())
        ptr->fAbove = m_oAboveAverage->GetValue();
    else
        ptr->fAbove = false;
    if(m_oBottom.IsInit())
        ptr->fBottom = m_oBottom->GetValue();
    else
        ptr->fBottom = false;
    if(m_oPercent.IsInit())
        ptr->fPercent = m_oPercent->GetValue();
    else
        ptr->fPercent = false;
    if(m_oText.IsInit())
        ptr->strParam = m_oText.get();
    else
        ptr->strParam.setSize(0xFFFFFFFF);

    if(!m_arrFormula.empty() && !m_arrFormula.front()->m_sText.empty())
    {
        ptr->rgce1 = m_arrFormula.front()->m_sText;
        m_arrFormula.erase(m_arrFormula.begin());
        ptr->cbFmla1 = 1;
    }
    else
    {
        ptr->cbFmla1 = 0;
    }
    if(!m_arrFormula.empty() && !m_arrFormula.front()->m_sText.empty())
    {
        ptr->rgce2 = m_arrFormula.front()->m_sText;
        m_arrFormula.erase(m_arrFormula.begin());
        ptr->cbFmla2 = 1;
    }
    else
    {
        ptr->cbFmla2 = 0;
    }
    if(!m_arrFormula.empty() && !m_arrFormula.front()->m_sText.empty())
    {
        ptr->rgce3 = m_arrFormula.front()->m_sText;
        m_arrFormula.erase(m_arrFormula.begin());
        ptr->cbFmla3 = 1;
    }
    else
    {
        ptr->cbFmla3 = 0;
    }

    ptr->iType = XLSB::CFType::CF_TYPE_EXPRIS;
    ptr->iParam =0;

    if (m_oType == SimpleTypes::Spreadsheet::ECfType::cellIs)
    {
        if (m_oOperator == SimpleTypes::Spreadsheet::ECfOperator::Operator_between)
            ptr->iParam = XLSB::CFOper::CF_OPER_BN;
        else if (m_oOperator == SimpleTypes::Spreadsheet::ECfOperator::Operator_notBetween)
            ptr->iParam = XLSB::CFOper::CF_OPER_NB;
        else if (m_oOperator == SimpleTypes::Spreadsheet::ECfOperator::Operator_equal)
            ptr->iParam = XLSB::CFOper::CF_OPER_EQ;
        else if (m_oOperator == SimpleTypes::Spreadsheet::ECfOperator::Operator_notEqual)
            ptr->iParam = XLSB::CFOper::CF_OPER_NE;
        else if (m_oOperator == SimpleTypes::Spreadsheet::ECfOperator::Operator_greaterThan)
            ptr->iParam = XLSB::CFOper::CF_OPER_GT;
        else if (m_oOperator == SimpleTypes::Spreadsheet::ECfOperator::Operator_lessThan)
            ptr->iParam = XLSB::CFOper::CF_OPER_LT;
        else if (m_oOperator == SimpleTypes::Spreadsheet::ECfOperator::Operator_greaterThanOrEqual)
            ptr->iParam = XLSB::CFOper::CF_OPER_GE;
        else if (m_oOperator == SimpleTypes::Spreadsheet::ECfOperator::Operator_lessThanOrEqual)
            ptr->iParam = XLSB::CFOper::CF_OPER_LE;
        ptr->iType = XLSB::CFType::CF_TYPE_CELLIS;
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_EXPR;
    }

    if (m_oType == SimpleTypes::Spreadsheet::ECfType::expression)
    {
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_FMLA;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::uniqueValues)
    {
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_UNIQUEVALUES;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::containsText)
    {
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_CONTAINSTEXT;
        ptr->iParam = XLSB::CFTextOper::CF_TEXTOPER_CONTAINS;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::notContainsText)
    {
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_CONTAINSTEXT;
        ptr->iParam = XLSB::CFTextOper::CF_TEXTOPER_NOTCONTAINS;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::beginsWith)
    {
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_CONTAINSTEXT;
        ptr->iParam = XLSB::CFTextOper::CF_TEXTOPER_BEGINSWITH;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::endsWith)
    {
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_CONTAINSTEXT;
        ptr->iParam = XLSB::CFTextOper::CF_TEXTOPER_ENDSWITH;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::containsBlanks)
    {
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_CONTAINSBLANKS;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::notContainsBlanks)
    {
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_CONTAINSNOBLANKS;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::containsErrors)
    {
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_CONTAINSERRORS;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::notContainsErrors)
    {
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_CONTAINSNOERRORS;
    }

    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::timePeriod)
    {
        if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::today)
        {
            ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODTODAY;
            ptr->iParam = XLSB::CFDateOper::CF_TIMEPERIOD_TODAY;
        }
        else if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::tomorrow)
        {
            ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODTOMORROW;
            ptr->iParam = XLSB::CFDateOper::CF_TIMEPERIOD_TOMORROW;
        }
        else if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::yesterday)
        {
            ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODYESTERDAY;
            ptr->iParam = XLSB::CFDateOper::CF_TIMEPERIOD_YESTERDAY;
        }
        else if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::last7Days)
        {
            ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODLAST7DAYS;
            ptr->iParam = XLSB::CFDateOper::CF_TIMEPERIOD_LAST7DAYS;
        }
        else if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::lastMonth)
        {
            ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODLASTMONTH;
            ptr->iParam = XLSB::CFDateOper::CF_TIMEPERIOD_LASTMONTH;
        }
        else if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::nextMonth)
        {
            ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODNEXTMONTH;
            ptr->iParam = XLSB::CFDateOper::CF_TIMEPERIOD_NEXTMONTH;
        }
        else if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::thisWeek)
        {
            ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODTHISWEEK;
            ptr->iParam = XLSB::CFDateOper::CF_TIMEPERIOD_THISWEEK;
        }
        else if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::nextWeek)
        {
            ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODNEXTWEEK;
            ptr->iParam = XLSB::CFDateOper::CF_TIMEPERIOD_NEXTWEEK;
        }
        else if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::lastWeek)
        {
            ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODLASTWEEK;
            ptr->iParam = XLSB::CFDateOper::CF_TIMEPERIOD_LASTWEEK;
        }
        else if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::thisMonth)
        {
            ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODTHISMONTH;
            ptr->iParam = XLSB::CFDateOper::CF_TIMEPERIOD_THISMONTH;
        }
    }

    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::aboveAverage)
    {
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_ABOVEAVERAGE;
        if(m_oStdDev.IsInit())
            ptr->iParam = m_oStdDev->GetValue();
        else
            ptr->iParam = 0;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::duplicateValues)
    {
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_DUPLICATEVALUES;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::colorScale)
    {
        ptr->iType = XLSB::CFType::CF_TYPE_GRADIENT;
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_GRADIENT;
        ptr->dxfId = 0xFFFFFFFF;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::dataBar)
    {
        ptr->iType = XLSB::CFType::CF_TYPE_DATABAR;
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_DATABAR;
        ptr->dxfId = 0xFFFFFFFF;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::iconSet)
    {
        ptr->iType = XLSB::CFType::CF_TYPE_MULTISTATE;
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_MULTISTATE;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::top10)
    {
        ptr->iType = XLSB::CFType::CF_TYPE_FILTER;
        if(m_oRank.IsInit())
            ptr->iParam = m_oRank->GetValue();
        else
            ptr->iParam = 1;
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_FILTER;
    }
    return objectPtr;
}
void CConditionalFormattingRule::WriteAttributes(XLS::CFRecordPtr& record, const  XLS::CellRef &cellRef)
{
    _UINT32 iType = XLSB::CFType::CF_TYPE_EXPRIS;
    _UINT32 iTemplate = XLSB::CFTemp::CF_TEMPLATE_EXPR;
    _UINT32 iParam = 0;
    _INT32 iPri = 1;
    _UINT32 dxfId = 0xFFFFFFFF;
    if(m_oPriority.IsInit())
        iPri = m_oPriority->GetValue();
    if(m_oDxfId.IsInit())
        dxfId = m_oDxfId->GetValue();
    if(!m_oType.IsInit())
        m_oType = SimpleTypes::Spreadsheet::ECfType::expression;


    if (m_oType == SimpleTypes::Spreadsheet::ECfType::cellIs)
    {
        if(m_oOperator.IsInit())
        {
            if (m_oOperator == SimpleTypes::Spreadsheet::ECfOperator::Operator_between)
                iParam = XLSB::CFOper::CF_OPER_BN;
            else if (m_oOperator == SimpleTypes::Spreadsheet::ECfOperator::Operator_notBetween)
                iParam = XLSB::CFOper::CF_OPER_NB;
            else if (m_oOperator == SimpleTypes::Spreadsheet::ECfOperator::Operator_equal)
                iParam = XLSB::CFOper::CF_OPER_EQ;
            else if (m_oOperator == SimpleTypes::Spreadsheet::ECfOperator::Operator_notEqual)
                iParam = XLSB::CFOper::CF_OPER_NE;
            else if (m_oOperator == SimpleTypes::Spreadsheet::ECfOperator::Operator_greaterThan)
                iParam = XLSB::CFOper::CF_OPER_GT;
            else if (m_oOperator == SimpleTypes::Spreadsheet::ECfOperator::Operator_lessThan)
                iParam = XLSB::CFOper::CF_OPER_LT;
            else if (m_oOperator == SimpleTypes::Spreadsheet::ECfOperator::Operator_greaterThanOrEqual)
                iParam = XLSB::CFOper::CF_OPER_GE;
            else if (m_oOperator == SimpleTypes::Spreadsheet::ECfOperator::Operator_lessThanOrEqual)
                iParam = XLSB::CFOper::CF_OPER_LE;
        }
        iType = XLSB::CFType::CF_TYPE_CELLIS;
        iTemplate = XLSB::CFTemp::CF_TEMPLATE_EXPR;
    }

    if (m_oType == SimpleTypes::Spreadsheet::ECfType::expression)
    {
        iTemplate = XLSB::CFTemp::CF_TEMPLATE_FMLA;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::uniqueValues)
    {
        iTemplate = XLSB::CFTemp::CF_TEMPLATE_UNIQUEVALUES;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::containsText)
    {
        iTemplate = XLSB::CFTemp::CF_TEMPLATE_CONTAINSTEXT;
        iParam = XLSB::CFTextOper::CF_TEXTOPER_CONTAINS;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::notContainsText)
    {
        iTemplate = XLSB::CFTemp::CF_TEMPLATE_CONTAINSTEXT;
        iParam = XLSB::CFTextOper::CF_TEXTOPER_NOTCONTAINS;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::beginsWith)
    {
        iTemplate = XLSB::CFTemp::CF_TEMPLATE_CONTAINSTEXT;
        iParam = XLSB::CFTextOper::CF_TEXTOPER_BEGINSWITH;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::endsWith)
    {
        iTemplate = XLSB::CFTemp::CF_TEMPLATE_CONTAINSTEXT;
        iParam = XLSB::CFTextOper::CF_TEXTOPER_ENDSWITH;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::containsBlanks)
    {
        iTemplate = XLSB::CFTemp::CF_TEMPLATE_CONTAINSBLANKS;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::notContainsBlanks)
    {
        iTemplate = XLSB::CFTemp::CF_TEMPLATE_CONTAINSNOBLANKS;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::containsErrors)
    {
        iTemplate = XLSB::CFTemp::CF_TEMPLATE_CONTAINSERRORS;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::notContainsErrors)
    {
        iTemplate = XLSB::CFTemp::CF_TEMPLATE_CONTAINSNOERRORS;
    }

    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::timePeriod)
    {
        if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::today)
        {
            iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODTODAY;
            iParam = XLSB::CFDateOper::CF_TIMEPERIOD_TODAY;
        }
        else if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::tomorrow)
        {
            iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODTOMORROW;
            iParam = XLSB::CFDateOper::CF_TIMEPERIOD_TOMORROW;
        }
        else if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::yesterday)
        {
            iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODYESTERDAY;
            iParam = XLSB::CFDateOper::CF_TIMEPERIOD_YESTERDAY;
        }
        else if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::last7Days)
        {
            iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODLAST7DAYS;
            iParam = XLSB::CFDateOper::CF_TIMEPERIOD_LAST7DAYS;
        }
        else if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::lastMonth)
        {
            iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODLASTMONTH;
            iParam = XLSB::CFDateOper::CF_TIMEPERIOD_LASTMONTH;
        }
        else if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::nextMonth)
        {
            iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODNEXTMONTH;
            iParam = XLSB::CFDateOper::CF_TIMEPERIOD_NEXTMONTH;
        }
        else if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::thisWeek)
        {
            iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODTHISWEEK;
            iParam = XLSB::CFDateOper::CF_TIMEPERIOD_THISWEEK;
        }
        else if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::nextWeek)
        {
            iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODNEXTWEEK;
            iParam = XLSB::CFDateOper::CF_TIMEPERIOD_NEXTWEEK;
        }
        else if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::lastWeek)
        {
            iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODLASTWEEK;
            iParam = XLSB::CFDateOper::CF_TIMEPERIOD_LASTWEEK;
        }
        else if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::thisMonth)
        {
            iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODTHISMONTH;
            iParam = XLSB::CFDateOper::CF_TIMEPERIOD_THISMONTH;
        }
    }

    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::aboveAverage)
    {
        iTemplate = XLSB::CFTemp::CF_TEMPLATE_ABOVEAVERAGE;
        if(m_oStdDev.IsInit())
            iParam = m_oStdDev->GetValue();
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::duplicateValues)
    {
        iTemplate = XLSB::CFTemp::CF_TEMPLATE_DUPLICATEVALUES;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::colorScale)
    {
        iType = XLSB::CFType::CF_TYPE_GRADIENT;
        iTemplate = XLSB::CFTemp::CF_TEMPLATE_GRADIENT;
        dxfId = 0xFFFFFFFF;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::dataBar)
    {
        iType = XLSB::CFType::CF_TYPE_DATABAR;
        iTemplate = XLSB::CFTemp::CF_TEMPLATE_DATABAR;
        dxfId = 0xFFFFFFFF;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::iconSet)
    {
        iType = XLSB::CFType::CF_TYPE_MULTISTATE;
        iTemplate = XLSB::CFTemp::CF_TEMPLATE_MULTISTATE;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::top10)
    {
        iType = XLSB::CFType::CF_TYPE_FILTER;
        if(m_oRank.IsInit())
            iParam = m_oRank->GetValue();
        else
            iParam = 1;
        iTemplate = XLSB::CFTemp::CF_TEMPLATE_FILTER;
    }
    *record << iType << iTemplate << dxfId << iPri << iParam;
    record->reserveNunBytes(8);
    _UINT16 flags = 0;
    if(m_oStopIfTrue.IsInit())
        SETBIT(flags, 1 , m_oStopIfTrue->GetValue())
    if(m_oAboveAverage.IsInit())
        SETBIT(flags, 2 , m_oAboveAverage->GetValue())
    if(m_oBottom.IsInit())
        SETBIT(flags, 3 , m_oBottom->GetValue())
    if(m_oPercent.IsInit())
        SETBIT(flags, 4 , m_oPercent->GetValue())
    *record << flags;
    {
        XLS::CFParsedFormula rgce1(cellRef);
        XLS::CFParsedFormula rgce2(cellRef);
        XLS::CFParsedFormula rgce3(cellRef);
        _UINT32 cbFmla1 = 0;
        _UINT32 cbFmla2 = 0;
        _UINT32 cbFmla3 = 0;
        if(!m_arrFormula.empty() && !m_arrFormula.front()->m_sText.empty())
        {
            rgce1 = m_arrFormula.front()->m_sText;
            m_arrFormula.erase(m_arrFormula.begin());
            cbFmla1 = 1;
        }

        if(!m_arrFormula.empty() && !m_arrFormula.front()->m_sText.empty())
        {
            rgce2 = m_arrFormula.front()->m_sText;
            m_arrFormula.erase(m_arrFormula.begin());
            cbFmla2 = 1;
        }

        if(!m_arrFormula.empty() && !m_arrFormula.front()->m_sText.empty())
        {
            rgce3 = m_arrFormula.front()->m_sText;
            m_arrFormula.erase(m_arrFormula.begin());
            cbFmla3 = 1;
        }
        XLSB::XLNullableWideString strParam;
        if(m_oText.IsInit())
            strParam = m_oText.get();
        else
            strParam.setSize(0xFFFFFFFF);
        *record << cbFmla1 << cbFmla2 << cbFmla3 << strParam;
        if(cbFmla1)
            *record << rgce1;
        if(cbFmla2)
            *record << rgce2;
        if(cbFmla3)
            *record << rgce3;
    }
}
XLS::BaseObjectPtr CConditionalFormattingRule::toBin14(const  XLS::CellRef &cellRef)
{
    auto ptr(new XLSB::CFRULE14());
    XLS::BaseObjectPtr objPtr(ptr);
    ptr->m_BrtBeginCFRule14 = WriteAttributes14(cellRef);
    if(m_oColorScale.IsInit())
    {
        ptr->m_source = m_oColorScale->toBin14();
    }
    else if(m_oDataBar.IsInit())
    {
        ptr->m_source = m_oDataBar->toBin14();
    }
    else if(m_oIconSet.IsInit())
    {
        ptr->m_source = m_oIconSet->toBin14();
    }
    return objPtr;
}

XLS::BaseObjectPtr CConditionalFormattingRule::WriteAttributes14(const  XLS::CellRef &cellRef)
{
    auto ptr(new XLSB::BeginCFRule14);
    XLS::BaseObjectPtr objPtr(ptr);
    if(m_oDxfId.IsInit())
    {
        ptr->dxfId = m_oDxfId->GetValue();
    }

    if(m_oPriority.IsInit())
        ptr->iPri = m_oPriority->GetValue();
    if(m_oStopIfTrue.IsInit())
        ptr->fStopTrue = m_oStopIfTrue->GetValue();
    if(m_oAboveAverage.IsInit())
        ptr->fAbove = m_oAboveAverage->GetValue();
    if(m_oBottom.IsInit())
        ptr->fBottom = m_oBottom->GetValue();
    if(m_oPercent.IsInit())
        ptr->fPercent = m_oPercent->GetValue();
    if(m_oId.IsInit())
    {
        ptr->fGuid = true;
        ptr->guid = m_oId.get();
    }
    if(m_oText.IsInit())
        ptr->strParam = m_oText.get();
    else
        ptr->strParam.setSize(0xFFFFFFFF);
    if(!m_arrFormula.empty())
    {
        for(auto i:m_arrFormula)
        {
            if(i.IsInit())
            {
                XLSB::FRTFormula tempFmla;
                tempFmla.formula.set_base_ref(cellRef);
                tempFmla.formula = i->m_sText;
                auto lastValType = GETBITS(tempFmla.formula.rgce.sequence.rbegin()->get()->ptg_id.get(),5,6);
                if(lastValType == 1 || lastValType == 3)
                {
                    SETBITS(tempFmla.formula.rgce.sequence.rbegin()->get()->ptg_id.get(),5,6,2);
                }
                ptr->FRTheader.rgFormulas.array.push_back(tempFmla);
            }
        }
        if(!ptr->FRTheader.rgFormulas.array.empty())
            ptr->FRTheader.fFormula = true;
      }
    ptr->iType = XLSB::CFType::CF_TYPE_EXPRIS;
    ptr->iParam =0;
    ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_EXPR;
    if(!m_oType.IsInit())
        return objPtr;
    if (m_oType == SimpleTypes::Spreadsheet::ECfType::cellIs)
    {
        if (m_oOperator == SimpleTypes::Spreadsheet::ECfOperator::Operator_between)
            ptr->iParam = XLSB::CFOper::CF_OPER_BN;
        else if (m_oOperator == SimpleTypes::Spreadsheet::ECfOperator::Operator_notBetween)
            ptr->iParam = XLSB::CFOper::CF_OPER_NB;
        else if (m_oOperator == SimpleTypes::Spreadsheet::ECfOperator::Operator_equal)
            ptr->iParam = XLSB::CFOper::CF_OPER_EQ;
        else if (m_oOperator == SimpleTypes::Spreadsheet::ECfOperator::Operator_notEqual)
            ptr->iParam = XLSB::CFOper::CF_OPER_NE;
        else if (m_oOperator == SimpleTypes::Spreadsheet::ECfOperator::Operator_greaterThan)
            ptr->iParam = XLSB::CFOper::CF_OPER_GT;
        else if (m_oOperator == SimpleTypes::Spreadsheet::ECfOperator::Operator_lessThan)
            ptr->iParam = XLSB::CFOper::CF_OPER_LT;
        else if (m_oOperator == SimpleTypes::Spreadsheet::ECfOperator::Operator_greaterThanOrEqual)
            ptr->iParam = XLSB::CFOper::CF_OPER_GE;
        else if (m_oOperator == SimpleTypes::Spreadsheet::ECfOperator::Operator_lessThanOrEqual)
            ptr->iParam = XLSB::CFOper::CF_OPER_LE;
        ptr->iType = XLSB::CFType::CF_TYPE_CELLIS;
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_EXPR;
    }

    if (m_oType == SimpleTypes::Spreadsheet::ECfType::expression)
    {
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_FMLA;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::uniqueValues)
    {
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_UNIQUEVALUES;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::containsText)
    {
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_CONTAINSTEXT;
        ptr->iParam = XLSB::CFTextOper::CF_TEXTOPER_CONTAINS;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::notContainsText)
    {
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_CONTAINSTEXT;
        ptr->iParam = XLSB::CFTextOper::CF_TEXTOPER_NOTCONTAINS;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::beginsWith)
    {
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_CONTAINSTEXT;
        ptr->iParam = XLSB::CFTextOper::CF_TEXTOPER_BEGINSWITH;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::endsWith)
    {
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_CONTAINSTEXT;
        ptr->iParam = XLSB::CFTextOper::CF_TEXTOPER_ENDSWITH;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::containsBlanks)
    {
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_CONTAINSBLANKS;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::notContainsBlanks)
    {
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_CONTAINSNOBLANKS;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::containsErrors)
    {
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_CONTAINSERRORS;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::notContainsErrors)
    {
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_CONTAINSNOERRORS;
    }

    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::timePeriod)
    {
        if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::today)
        {
            ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODTODAY;
            ptr->iParam = XLSB::CFDateOper::CF_TIMEPERIOD_TODAY;
        }
        else if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::tomorrow)
        {
            ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODTOMORROW;
            ptr->iParam = XLSB::CFDateOper::CF_TIMEPERIOD_TOMORROW;
        }
        else if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::yesterday)
        {
            ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODYESTERDAY;
            ptr->iParam = XLSB::CFDateOper::CF_TIMEPERIOD_YESTERDAY;
        }
        else if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::last7Days)
        {
            ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODLAST7DAYS;
            ptr->iParam = XLSB::CFDateOper::CF_TIMEPERIOD_LAST7DAYS;
        }
        else if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::lastMonth)
        {
            ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODLASTMONTH;
            ptr->iParam = XLSB::CFDateOper::CF_TIMEPERIOD_LASTMONTH;
        }
        else if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::nextMonth)
        {
            ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODNEXTMONTH;
            ptr->iParam = XLSB::CFDateOper::CF_TIMEPERIOD_NEXTMONTH;
        }
        else if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::thisWeek)
        {
            ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODTHISWEEK;
            ptr->iParam = XLSB::CFDateOper::CF_TIMEPERIOD_THISWEEK;
        }
        else if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::nextWeek)
        {
            ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODNEXTWEEK;
            ptr->iParam = XLSB::CFDateOper::CF_TIMEPERIOD_NEXTWEEK;
        }
        else if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::lastWeek)
        {
            ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODLASTWEEK;
            ptr->iParam = XLSB::CFDateOper::CF_TIMEPERIOD_LASTWEEK;
        }
        else if (m_oTimePeriod == SimpleTypes::Spreadsheet::ETimePeriod::thisMonth)
        {
            ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODTHISMONTH;
            ptr->iParam = XLSB::CFDateOper::CF_TIMEPERIOD_THISMONTH;
        }
    }

    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::aboveAverage)
    {
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_ABOVEAVERAGE;
        if(m_oStdDev.IsInit())
            ptr->iParam = m_oStdDev->GetValue();
        else
            ptr->iParam = 0;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::duplicateValues)
    {
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_DUPLICATEVALUES;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::colorScale)
    {
        ptr->iType = XLSB::CFType::CF_TYPE_GRADIENT;
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_GRADIENT;
        ptr->dxfId = 0xFFFFFFFF;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::dataBar)
    {
        ptr->iType = XLSB::CFType::CF_TYPE_DATABAR;
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_DATABAR;
        ptr->dxfId = 0xFFFFFFFF;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::iconSet)
    {
        ptr->iType = XLSB::CFType::CF_TYPE_MULTISTATE;
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_MULTISTATE;
    }
    else if (m_oType == SimpleTypes::Spreadsheet::ECfType::top10)
    {
        ptr->iType = XLSB::CFType::CF_TYPE_FILTER;
        if(m_oRank.IsInit())
            ptr->iParam = m_oRank->GetValue();
        else
            ptr->iParam = 1;
        ptr->iTemplate = XLSB::CFTemp::CF_TEMPLATE_FILTER;
    }

    if(!ptr->FRTheader.rgFormulas.array.empty())
    {
    if(ptr->iType != XLSB::CFType::CF_TYPE_GRADIENT && ptr->iType != XLSB::CFType::CF_TYPE_DATABAR
        && ptr->iType != XLSB::CFType::CF_TYPE_MULTISTATE)
            {
                ptr->cbFmla1 = 1;
                ptr->cbFmla3 = 0;
            }
        else
            {
                ptr->cbFmla1 = 0;
                ptr->cbFmla3 = 1;
            }
        if(m_arrFormula.size() > 1 && ptr->cbFmla1)
            ptr->cbFmla2 = 1;
    }
    return objPtr;
}

template<typename Type>
nullable<Type> CConditionalFormattingRule::Merge(const nullable<Type> &oPrev, const nullable<Type> &oCurrent)
{
	nullable<Type> oResult;

	if ( oCurrent.IsInit() )	oResult = oCurrent;
	else if ( oPrev.IsInit() )	oResult = oPrev;

	return oResult;
}
const CConditionalFormattingRule CConditionalFormattingRule::Merge(const CConditionalFormattingRule& oPrev, const CConditionalFormattingRule& oCurrent)
{
	CConditionalFormattingRule oRule;

	oRule.m_oAboveAverage	= Merge( oPrev.m_oAboveAverage,	oCurrent.m_oAboveAverage );
	oRule.m_oBottom			= Merge( oPrev.m_oBottom,		oCurrent.m_oBottom );
	oRule.m_oDxfId			= Merge( oPrev.m_oDxfId,		oCurrent.m_oDxfId );
	oRule.m_oEqualAverage	= Merge( oPrev.m_oEqualAverage,	oCurrent.m_oEqualAverage );
	oRule.m_oOperator		= Merge( oPrev.m_oOperator,		oCurrent.m_oOperator );
	oRule.m_oPercent		= Merge( oPrev.m_oPercent,		oCurrent.m_oPercent );
	oRule.m_oPriority		= Merge( oPrev.m_oPriority,		oCurrent.m_oPriority );
	oRule.m_oRank			= Merge( oPrev.m_oRank,			oCurrent.m_oRank );
	oRule.m_oStdDev			= Merge( oPrev.m_oStdDev,		oCurrent.m_oStdDev );
	oRule.m_oStopIfTrue		= Merge( oPrev.m_oStopIfTrue,	oCurrent.m_oStopIfTrue );
	oRule.m_oText			= Merge( oPrev.m_oText,			oCurrent.m_oText );
	oRule.m_oTimePeriod		= Merge( oPrev.m_oTimePeriod,	oCurrent.m_oTimePeriod );
	oRule.m_oType			= Merge( oPrev.m_oType,			oCurrent.m_oType );
	oRule.m_oDxf			= Merge( oPrev.m_oDxf,			oCurrent.m_oDxf );

	if (oRule.m_oDxf.IsInit() && oRule.m_oDxfId.IsInit())
	{
		oRule.m_oDxfId.reset();
	}

	if (oPrev.m_oIconSet.IsInit() && oCurrent.m_oIconSet.IsInit())
		oRule.m_oIconSet	= CIconSet::Merge	( oPrev.m_oIconSet.get(),		oCurrent.m_oIconSet.get());
	else
		oRule.m_oIconSet	= Merge( oPrev.m_oIconSet,	oCurrent.m_oIconSet );

	if (oPrev.m_oColorScale.IsInit() && oCurrent.m_oColorScale.IsInit())
		oRule.m_oColorScale = CColorScale::Merge( oPrev.m_oColorScale.get(),	oCurrent.m_oColorScale.get());
	else
		oRule.m_oColorScale	= Merge( oPrev.m_oColorScale,	oCurrent.m_oColorScale );

	if (oPrev.m_oDataBar.IsInit() && oCurrent.m_oDataBar.IsInit())
		oRule.m_oDataBar	= CDataBar::Merge	( oPrev.m_oDataBar.get(),		oCurrent.m_oDataBar.get());
	else
		oRule.m_oDataBar	= Merge( oPrev.m_oDataBar,	oCurrent.m_oDataBar );

	for (size_t i = 0; i < oCurrent.m_arrFormula.size(); i++)
	{
		oRule.m_arrFormula.push_back(oCurrent.m_arrFormula[i]);
	}
	for (size_t i = oCurrent.m_arrFormula.size(); i < oPrev.m_arrFormula.size(); i++)
	{
		oRule.m_arrFormula.push_back(oPrev.m_arrFormula[i]);
	}

	return oRule;
}
void CConditionalFormattingRule::ReadAttributes(XmlUtils::CXmlLiteReader& oReader)
{
	WritingElement_ReadAttributes_Start(oReader)

    WritingElement_ReadAttributes_Read_if		(oReader, L"aboveAverage"	, m_oAboveAverage) //
    WritingElement_ReadAttributes_Read_else_if	(oReader, L"bottom"			, m_oBottom) //
    WritingElement_ReadAttributes_Read_else_if	(oReader, L"dxfId"			, m_oDxfId) //
    WritingElement_ReadAttributes_Read_else_if	(oReader, L"equalAverage"	, m_oEqualAverage) //
    WritingElement_ReadAttributes_Read_else_if	(oReader, L"operator"		, m_oOperator) //
    WritingElement_ReadAttributes_Read_else_if	(oReader, L"percent"		, m_oPercent) //
    WritingElement_ReadAttributes_Read_else_if	(oReader, L"priority"		, m_oPriority) //
    WritingElement_ReadAttributes_Read_else_if	(oReader, L"rank"			, m_oRank) //
    WritingElement_ReadAttributes_Read_else_if	(oReader, L"stdDev"			, m_oStdDev) //
    WritingElement_ReadAttributes_Read_else_if	(oReader, L"stopIfTrue"		, m_oStopIfTrue) //
    WritingElement_ReadAttributes_Read_else_if	(oReader, L"text"			, m_oText) //
    WritingElement_ReadAttributes_Read_else_if	(oReader, L"timePeriod"		, m_oTimePeriod) //
    WritingElement_ReadAttributes_Read_else_if	(oReader, L"type"			, m_oType) //
	WritingElement_ReadAttributes_Read_else_if	(oReader, L"id"				, m_oId)

	WritingElement_ReadAttributes_End(oReader)
}
void CConditionalFormattingRule::ReadAttributes(XLS::BaseObjectPtr& obj)
{
    if(obj->get_type() == XLS::typeBeginCFRule)
    {
        auto ptr = static_cast<XLSB::BeginCFRule*>(obj.get());
        if(ptr != nullptr)
        {
            switch (ptr->iType.value().get())
            {
                case XLSB::CFType::CF_TYPE_CELLIS:
                {
                    m_oType = SimpleTypes::Spreadsheet::ECfType::cellIs;
                    switch (ptr->iParam)
                    {
                        case XLSB::CFOper::CF_OPER_BN:
                            m_oOperator = SimpleTypes::Spreadsheet::ECfOperator::Operator_between;
                            break;
                        case XLSB::CFOper::CF_OPER_NB:
                            m_oOperator = SimpleTypes::Spreadsheet::ECfOperator::Operator_notBetween;
                            break;
                        case XLSB::CFOper::CF_OPER_EQ:
                            m_oOperator = SimpleTypes::Spreadsheet::ECfOperator::Operator_equal;
                            break;
                        case XLSB::CFOper::CF_OPER_NE:
                            m_oOperator = SimpleTypes::Spreadsheet::ECfOperator::Operator_notEqual;
                            break;
                        case XLSB::CFOper::CF_OPER_GT:
                            m_oOperator = SimpleTypes::Spreadsheet::ECfOperator::Operator_greaterThan;
                            break;
                        case XLSB::CFOper::CF_OPER_LT:
                            m_oOperator = SimpleTypes::Spreadsheet::ECfOperator::Operator_lessThan;
                            break;
                        case XLSB::CFOper::CF_OPER_GE:
                            m_oOperator = SimpleTypes::Spreadsheet::ECfOperator::Operator_greaterThanOrEqual;
                            break;
                        case XLSB::CFOper::CF_OPER_LE:
                            m_oOperator = SimpleTypes::Spreadsheet::ECfOperator::Operator_lessThanOrEqual;
                            break;
                    }
                    break;
                }
                case XLSB::CFType::CF_TYPE_EXPRIS:
                {
                    switch (ptr->iTemplate.value().get())
                    {
                        case XLSB::CFTemp::CF_TEMPLATE_FMLA:
                        {
                            m_oType = SimpleTypes::Spreadsheet::ECfType::expression;
                            break;
                        }
                        case XLSB::CFTemp::CF_TEMPLATE_UNIQUEVALUES:
                        {
                            m_oType = SimpleTypes::Spreadsheet::ECfType::uniqueValues;
                            break;
                        }
                        case XLSB::CFTemp::CF_TEMPLATE_EQUALABOVEAVERAGE:
                        case XLSB::CFTemp::CF_TEMPLATE_EQUALBELOWAVERAGE:
                        {
                            m_oEqualAverage = true;
                            break;
                        }
                        case XLSB::CFTemp::CF_TEMPLATE_CONTAINSTEXT:
                        {
                            switch (ptr->iParam)
                            {
                                case XLSB::CFTextOper::CF_TEXTOPER_CONTAINS:
                                    m_oType     = SimpleTypes::Spreadsheet::ECfType::containsText;
                                    m_oOperator = SimpleTypes::Spreadsheet::ECfOperator::Operator_containsText;
                                    break;
                                case XLSB::CFTextOper::CF_TEXTOPER_NOTCONTAINS:
                                    m_oType     = SimpleTypes::Spreadsheet::ECfType::notContainsText;
                                    m_oOperator = SimpleTypes::Spreadsheet::ECfOperator::Operator_notContains;
                                    break;
                                case XLSB::CFTextOper::CF_TEXTOPER_BEGINSWITH:
                                    m_oType     = SimpleTypes::Spreadsheet::ECfType::beginsWith;
                                    m_oOperator = SimpleTypes::Spreadsheet::ECfOperator::Operator_beginsWith;
                                    break;
                                case XLSB::CFTextOper::CF_TEXTOPER_ENDSWITH:
                                    m_oType     = SimpleTypes::Spreadsheet::ECfType::endsWith;
                                    m_oOperator = SimpleTypes::Spreadsheet::ECfOperator::Operator_endsWith;
                                    break;
                            }
                            break;
                        }
                        case XLSB::CFTemp::CF_TEMPLATE_CONTAINSBLANKS:
                        {
                            m_oType = SimpleTypes::Spreadsheet::ECfType::containsBlanks;
                            break;
                        }
                        case XLSB::CFTemp::CF_TEMPLATE_CONTAINSNOBLANKS:
                        {
                            m_oType = SimpleTypes::Spreadsheet::ECfType::notContainsBlanks;
                            break;
                        }
                        case XLSB::CFTemp::CF_TEMPLATE_CONTAINSERRORS:
                        {
                            m_oType = SimpleTypes::Spreadsheet::ECfType::containsErrors;
                            break;
                        }
                        case XLSB::CFTemp::CF_TEMPLATE_CONTAINSNOERRORS:
                        {
                            m_oType = SimpleTypes::Spreadsheet::ECfType::notContainsErrors;
                            break;
                        }
                        case XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODTODAY:
                        case XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODTOMORROW:
                        case XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODYESTERDAY:
                        case XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODLAST7DAYS:
                        case XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODLASTMONTH:
                        case XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODNEXTMONTH:
                        case XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODTHISWEEK:
                        case XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODNEXTWEEK:
                        case XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODLASTWEEK:
                        case XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODTHISMONTH:
                        {
                            m_oType = SimpleTypes::Spreadsheet::ECfType::timePeriod;
                            switch (ptr->iParam)
                            {
                                case XLSB::CFDateOper::CF_TIMEPERIOD_TODAY:
                                    m_oTimePeriod = SimpleTypes::Spreadsheet::ETimePeriod::today;
                                    break;
                                case XLSB::CFDateOper::CF_TIMEPERIOD_YESTERDAY:
                                    m_oTimePeriod = SimpleTypes::Spreadsheet::ETimePeriod::yesterday;
                                    break;
                                case XLSB::CFDateOper::CF_TIMEPERIOD_LAST7DAYS:
                                    m_oTimePeriod = SimpleTypes::Spreadsheet::ETimePeriod::last7Days;
                                    break;
                                case XLSB::CFDateOper::CF_TIMEPERIOD_THISWEEK:
                                    m_oTimePeriod = SimpleTypes::Spreadsheet::ETimePeriod::thisWeek;
                                    break;
                                case XLSB::CFDateOper::CF_TIMEPERIOD_LASTWEEK:
                                    m_oTimePeriod = SimpleTypes::Spreadsheet::ETimePeriod::lastWeek;
                                    break;
                                case XLSB::CFDateOper::CF_TIMEPERIOD_LASTMONTH:
                                    m_oTimePeriod = SimpleTypes::Spreadsheet::ETimePeriod::lastMonth;
                                    break;
                                case XLSB::CFDateOper::CF_TIMEPERIOD_TOMORROW:
                                    m_oTimePeriod = SimpleTypes::Spreadsheet::ETimePeriod::tomorrow;
                                    break;
                                case XLSB::CFDateOper::CF_TIMEPERIOD_NEXTWEEK:
                                    m_oTimePeriod = SimpleTypes::Spreadsheet::ETimePeriod::nextWeek;
                                    break;
                                case XLSB::CFDateOper::CF_TIMEPERIOD_NEXTMONTH:
                                    m_oTimePeriod = SimpleTypes::Spreadsheet::ETimePeriod::nextMonth;
                                    break;
                                case XLSB::CFDateOper::CF_TIMEPERIOD_THISMONTH:
                                    m_oTimePeriod = SimpleTypes::Spreadsheet::ETimePeriod::thisMonth;
                                    break;
                            }
                            break;
                        }
                        case XLSB::CFTemp::CF_TEMPLATE_ABOVEAVERAGE:
                        case XLSB::CFTemp::CF_TEMPLATE_BELOWAVERAGE:
                        {
                            m_oType = SimpleTypes::Spreadsheet::ECfType::aboveAverage;
                            m_oStdDev = ptr->iParam;
                            break;
                        }
                        case XLSB::CFTemp::CF_TEMPLATE_DUPLICATEVALUES:
                        {
                            m_oType = SimpleTypes::Spreadsheet::ECfType::duplicateValues;
                            break;
                        }

                    }
                    break;
                }
                case XLSB::CFType::CF_TYPE_GRADIENT:
                {
                    m_oType = SimpleTypes::Spreadsheet::ECfType::colorScale;
                    break;
                }
                case XLSB::CFType::CF_TYPE_DATABAR:
                {
                    m_oType = SimpleTypes::Spreadsheet::ECfType::dataBar;
                    break;
                }
                case XLSB::CFType::CF_TYPE_MULTISTATE:
                {
                    m_oType = SimpleTypes::Spreadsheet::ECfType::iconSet;
                    break;
                }
                case XLSB::CFType::CF_TYPE_FILTER:
                {
                    m_oType = SimpleTypes::Spreadsheet::ECfType::top10;
                    m_oRank = ptr->iParam;
                    break;
                }
            }

            if(ptr->iType.value().get() != XLSB::CFType::CF_TYPE_GRADIENT
                    && ptr->iType.value().get() != XLSB::CFType::CF_TYPE_DATABAR
                    && ptr->iType.value().get() != XLSB::CFType::CF_TYPE_MULTISTATE
                    && ptr->dxfId != 0xFFFFFFFF)
            {
                m_oDxfId           = ptr->dxfId;
            }
            m_oPriority        = ptr->iPri;
            m_oStopIfTrue      = ptr->fStopTrue;
            m_oAboveAverage    = ptr->fAbove;
            m_oBottom          = ptr->fBottom;
            m_oPercent         = ptr->fPercent;
            m_oText            = ptr->strParam.value();

            if(ptr->cbFmla1)
            {
                nullable<CFormulaCF> formula1;
                formula1.Init();
                formula1->m_sNodeName = L"formula";
                formula1->m_sText = ptr->rgce1.getAssembledFormula();
                m_arrFormula.push_back(formula1);
            }
            if(ptr->cbFmla2)
            {
                nullable<CFormulaCF> formula2;
                formula2.Init();
                formula2->m_sNodeName = L"formula";
                formula2->m_sText = ptr->rgce2.getAssembledFormula();
                m_arrFormula.push_back(formula2);
            }
            if(ptr->cbFmla3)
            {
                nullable<CFormulaCF> formula3;
                formula3.Init();
                formula3->m_sNodeName = L"formula";
                formula3->m_sText = ptr->rgce3.getAssembledFormula();
                m_arrFormula.push_back(formula3);
            }
        }
    }
    else if(obj->get_type() == XLS::typeBeginCFRule14)
    {
        auto ptr = static_cast<XLSB::BeginCFRule14*>(obj.get());
        if(ptr != nullptr)
        {
            switch (ptr->iType.value().get())
            {
                case XLSB::CFType::CF_TYPE_CELLIS:
                {
                    m_oType = SimpleTypes::Spreadsheet::ECfType::cellIs;
                    switch (ptr->iParam)
                    {
                        case XLSB::CFOper::CF_OPER_BN:
                            m_oOperator = SimpleTypes::Spreadsheet::ECfOperator::Operator_between;
                            break;
                        case XLSB::CFOper::CF_OPER_NB:
                            m_oOperator = SimpleTypes::Spreadsheet::ECfOperator::Operator_notBetween;
                            break;
                        case XLSB::CFOper::CF_OPER_EQ:
                            m_oOperator = SimpleTypes::Spreadsheet::ECfOperator::Operator_equal;
                            break;
                        case XLSB::CFOper::CF_OPER_NE:
                            m_oOperator = SimpleTypes::Spreadsheet::ECfOperator::Operator_notEqual;
                            break;
                        case XLSB::CFOper::CF_OPER_GT:
                            m_oOperator = SimpleTypes::Spreadsheet::ECfOperator::Operator_greaterThan;
                            break;
                        case XLSB::CFOper::CF_OPER_LT:
                            m_oOperator = SimpleTypes::Spreadsheet::ECfOperator::Operator_lessThan;
                            break;
                        case XLSB::CFOper::CF_OPER_GE:
                            m_oOperator = SimpleTypes::Spreadsheet::ECfOperator::Operator_greaterThanOrEqual;
                            break;
                        case XLSB::CFOper::CF_OPER_LE:
                            m_oOperator = SimpleTypes::Spreadsheet::ECfOperator::Operator_lessThanOrEqual;
                            break;
                    }
                    break;
                }
                case XLSB::CFType::CF_TYPE_EXPRIS:
                {
                    switch (ptr->iTemplate.value().get())
                    {
                        case XLSB::CFTemp::CF_TEMPLATE_FMLA:
                        {
                            m_oType = SimpleTypes::Spreadsheet::ECfType::expression;
                            break;
                        }
                        case XLSB::CFTemp::CF_TEMPLATE_UNIQUEVALUES:
                        {
                            m_oType = SimpleTypes::Spreadsheet::ECfType::uniqueValues;
                            break;
                        }
                        case XLSB::CFTemp::CF_TEMPLATE_EQUALABOVEAVERAGE:
                        case XLSB::CFTemp::CF_TEMPLATE_EQUALBELOWAVERAGE:
                        {
                            m_oEqualAverage = true;
                            break;
                        }
                        case XLSB::CFTemp::CF_TEMPLATE_CONTAINSTEXT:
                        {
                            switch (ptr->iParam)
                            {
                                case XLSB::CFTextOper::CF_TEXTOPER_CONTAINS:
                                    m_oType     = SimpleTypes::Spreadsheet::ECfType::containsText;
                                    m_oOperator = SimpleTypes::Spreadsheet::ECfOperator::Operator_containsText;
                                    break;
                                case XLSB::CFTextOper::CF_TEXTOPER_NOTCONTAINS:
                                    m_oType     = SimpleTypes::Spreadsheet::ECfType::notContainsText;
                                    m_oOperator = SimpleTypes::Spreadsheet::ECfOperator::Operator_notContains;
                                    break;
                                case XLSB::CFTextOper::CF_TEXTOPER_BEGINSWITH:
                                    m_oType     = SimpleTypes::Spreadsheet::ECfType::beginsWith;
                                    m_oOperator = SimpleTypes::Spreadsheet::ECfOperator::Operator_beginsWith;
                                    break;
                                case XLSB::CFTextOper::CF_TEXTOPER_ENDSWITH:
                                    m_oType     = SimpleTypes::Spreadsheet::ECfType::endsWith;
                                    m_oOperator = SimpleTypes::Spreadsheet::ECfOperator::Operator_endsWith;
                                    break;
                            }
                            break;
                        }
                        case XLSB::CFTemp::CF_TEMPLATE_CONTAINSBLANKS:
                        {
                            m_oType = SimpleTypes::Spreadsheet::ECfType::containsBlanks;
                            break;
                        }
                        case XLSB::CFTemp::CF_TEMPLATE_CONTAINSNOBLANKS:
                        {
                            m_oType = SimpleTypes::Spreadsheet::ECfType::notContainsBlanks;
                            break;
                        }
                        case XLSB::CFTemp::CF_TEMPLATE_CONTAINSERRORS:
                        {
                            m_oType = SimpleTypes::Spreadsheet::ECfType::containsErrors;
                            break;
                        }
                        case XLSB::CFTemp::CF_TEMPLATE_CONTAINSNOERRORS:
                        {
                            m_oType = SimpleTypes::Spreadsheet::ECfType::notContainsErrors;
                            break;
                        }
                        case XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODTODAY:
                        case XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODTOMORROW:
                        case XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODYESTERDAY:
                        case XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODLAST7DAYS:
                        case XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODLASTMONTH:
                        case XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODNEXTMONTH:
                        case XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODTHISWEEK:
                        case XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODNEXTWEEK:
                        case XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODLASTWEEK:
                        case XLSB::CFTemp::CF_TEMPLATE_TIMEPERIODTHISMONTH:
                        {
                            m_oType = SimpleTypes::Spreadsheet::ECfType::timePeriod;
                            switch (ptr->iParam)
                            {
                                case XLSB::CFDateOper::CF_TIMEPERIOD_TODAY:
                                    m_oTimePeriod = SimpleTypes::Spreadsheet::ETimePeriod::today;
                                    break;
                                case XLSB::CFDateOper::CF_TIMEPERIOD_YESTERDAY:
                                    m_oTimePeriod = SimpleTypes::Spreadsheet::ETimePeriod::yesterday;
                                    break;
                                case XLSB::CFDateOper::CF_TIMEPERIOD_LAST7DAYS:
                                    m_oTimePeriod = SimpleTypes::Spreadsheet::ETimePeriod::last7Days;
                                    break;
                                case XLSB::CFDateOper::CF_TIMEPERIOD_THISWEEK:
                                    m_oTimePeriod = SimpleTypes::Spreadsheet::ETimePeriod::thisWeek;
                                    break;
                                case XLSB::CFDateOper::CF_TIMEPERIOD_LASTWEEK:
                                    m_oTimePeriod = SimpleTypes::Spreadsheet::ETimePeriod::lastWeek;
                                    break;
                                case XLSB::CFDateOper::CF_TIMEPERIOD_LASTMONTH:
                                    m_oTimePeriod = SimpleTypes::Spreadsheet::ETimePeriod::lastMonth;
                                    break;
                                case XLSB::CFDateOper::CF_TIMEPERIOD_TOMORROW:
                                    m_oTimePeriod = SimpleTypes::Spreadsheet::ETimePeriod::tomorrow;
                                    break;
                                case XLSB::CFDateOper::CF_TIMEPERIOD_NEXTWEEK:
                                    m_oTimePeriod = SimpleTypes::Spreadsheet::ETimePeriod::nextWeek;
                                    break;
                                case XLSB::CFDateOper::CF_TIMEPERIOD_NEXTMONTH:
                                    m_oTimePeriod = SimpleTypes::Spreadsheet::ETimePeriod::nextMonth;
                                    break;
                                case XLSB::CFDateOper::CF_TIMEPERIOD_THISMONTH:
                                    m_oTimePeriod = SimpleTypes::Spreadsheet::ETimePeriod::thisMonth;
                                    break;
                            }
                            break;
                        }
                        case XLSB::CFTemp::CF_TEMPLATE_ABOVEAVERAGE:
                        case XLSB::CFTemp::CF_TEMPLATE_BELOWAVERAGE:
                        {
                            m_oType = SimpleTypes::Spreadsheet::ECfType::aboveAverage;
                            m_oStdDev = ptr->iParam;
                            break;
                        }
                        case XLSB::CFTemp::CF_TEMPLATE_DUPLICATEVALUES:
                        {
                            m_oType = SimpleTypes::Spreadsheet::ECfType::duplicateValues;
                            break;
                        }

                    }
                    break;
                }
                case XLSB::CFType::CF_TYPE_GRADIENT:
                {
                    m_oType = SimpleTypes::Spreadsheet::ECfType::colorScale;
                    break;
                }
                case XLSB::CFType::CF_TYPE_DATABAR:
                {
                    m_oType = SimpleTypes::Spreadsheet::ECfType::dataBar;
                    break;
                }
                case XLSB::CFType::CF_TYPE_MULTISTATE:
                {
                    m_oType = SimpleTypes::Spreadsheet::ECfType::iconSet;
                    break;
                }
                case XLSB::CFType::CF_TYPE_FILTER:
                {
                    m_oType = SimpleTypes::Spreadsheet::ECfType::top10;
                    m_oRank = ptr->iParam;
                    break;
                }
            }

            if(ptr->iType.value().get() != XLSB::CFType::CF_TYPE_GRADIENT
                    && ptr->iType.value().get() != XLSB::CFType::CF_TYPE_DATABAR
                    && ptr->iType.value().get() != XLSB::CFType::CF_TYPE_MULTISTATE
                    && ptr->dxfId != 0xFFFFFFFF)
            {
                m_oDxfId           = ptr->dxfId;
            }
            m_oPriority        = ptr->iPri;
            m_oStopIfTrue      = ptr->fStopTrue;
            m_oAboveAverage    = ptr->fAbove;
            m_oBottom          = ptr->fBottom;
            m_oPercent         = ptr->fPercent;
            m_oText            = ptr->strParam.value();
            m_oId              = ptr->guid;

            if(ptr->cbFmla1 && ptr->FRTheader.rgFormulas.array.size() > 0)
            {
                nullable<CFormulaCF> formula1;
                formula1.Init();
                formula1->m_sNodeName = L"formula";
                formula1->m_sText = ptr->FRTheader.rgFormulas.array[0].formula.getAssembledFormula();
                m_arrFormula.push_back(formula1);

                if(ptr->cbFmla2 && ptr->FRTheader.rgFormulas.array.size() > 1)
                {
                    nullable<CFormulaCF> formula2;
                    formula2.Init();
                    formula2->m_sNodeName = L"formula";
                    formula2->m_sText = ptr->FRTheader.rgFormulas.array[1].formula.getAssembledFormula();
                    m_arrFormula.push_back(formula2);
                }
            }

            else if(ptr->cbFmla3 && ptr->FRTheader.rgFormulas.array.size() > 0)
            {
                nullable<CFormulaCF> formula3;
                formula3.Init();
                formula3->m_sNodeName = L"formula";
                formula3->m_sText = ptr->FRTheader.rgFormulas.array[0].formula.getAssembledFormula();
                m_arrFormula.push_back(formula3);
            }
        }
    }

}

CConditionalFormatting::CConditionalFormatting(OOX::Document *pMain) : WritingElementWithChilds<CConditionalFormattingRule>(pMain), m_bIsExtended(false), m_bIsValid(false)
{
}
void CConditionalFormatting::fromXML(XmlUtils::CXmlNode& node)
{
}
std::wstring CConditionalFormatting::toXML() const
{
	return L"";
}
void CConditionalFormatting::toXML(NSStringUtils::CStringBuilder& writer) const
{
	if (m_bIsValid == false) return;

	if (m_bIsExtended == false)
		toXML2(writer, false);
}
EElementType CConditionalFormatting::getType () const
{
	return et_x_ConditionalFormatting;
}
bool CConditionalFormatting::IsExtended()
{
	m_bIsExtended = false;
	m_bIsValid = false;

	if (m_oSqRef.IsInit() == false) return m_bIsExtended;

	m_bIsValid = true;

    for ( size_t i = 0; i < m_arrItems.size(); ++i)
	{
        if ( !m_arrItems[i]) continue;

		if (m_arrItems[i]->isValid() == false)
			m_bIsValid = false;
		if (m_arrItems[i]->isExtended() == true)
			m_bIsExtended = true;
	}
	return m_bIsExtended;
}
void CConditionalFormatting::toXML2(NSStringUtils::CStringBuilder& writer, bool bExtendedWrite) const
{
	std::wstring node_name = bExtendedWrite ? L"x14:conditionalFormatting" : L"conditionalFormatting";

	writer.WriteString(L"<" + node_name);
		if (bExtendedWrite)
		{
			WritingStringAttrString(L"xmlns:xm", L"http://schemas.microsoft.com/office/excel/2006/main");
		}
		else
		{
			WritingStringAttrString(L"sqref", m_oSqRef.get());
		}
		if (m_oPivot.IsInit() && true == m_oPivot->ToBool())
		{
			writer.WriteString(L" pivot=\"1\"");
		}
	writer.WriteString(L">");

    for ( size_t i = 0; i < m_arrItems.size(); ++i)
    {
        if (  m_arrItems[i] )
        {
            m_arrItems[i]->toXML2(writer, bExtendedWrite);
        }
    }
	if (bExtendedWrite)
	{
		writer.WriteString(L"<xm:sqref>" + m_oSqRef.get() + L"</xm:sqref>");
	}
	writer.WriteString(L"</" + node_name + L">");
}
void CConditionalFormatting::fromXML(XmlUtils::CXmlLiteReader& oReader)
{
	ReadAttributes(oReader);

	if (oReader.IsEmptyNode())
		return;

	int nCurDepth = oReader.GetDepth();
	while (oReader.ReadNextSiblingNode(nCurDepth))
	{
		std::wstring sName = XmlUtils::GetNameNoNS(oReader.GetName());

		if (L"cfRule" == sName)
		{
			CConditionalFormattingRule* pConditionalFormattingRule = new CConditionalFormattingRule();
			*pConditionalFormattingRule = oReader;
			m_arrItems.push_back(pConditionalFormattingRule);
		}
		if (L"sqref" == sName || L"Range" == sName)
			m_oSqRef = oReader.GetText2();
	}
}
void CConditionalFormatting::fromBin(XLS::BaseObjectPtr& obj)
{
    if(obj->get_type() == XLS::typeCONDITIONALFORMATTING)
    {
        auto ptr = static_cast<XLSB::CONDITIONALFORMATTING*>(obj.get());

        ReadAttributes(ptr->m_BrtBeginConditionalFormatting);

        if(ptr->m_arCFRULE.empty())
            return;

        for(auto &pCFRULE: ptr->m_arCFRULE)
            m_arrItems.push_back(new CConditionalFormattingRule(pCFRULE));
    }
    else if(obj->get_type() == XLS::typeCONDITIONALFORMATTING14)
    {
        auto ptr = static_cast<XLSB::CONDITIONALFORMATTING14*>(obj.get());

        ReadAttributes(ptr->m_BrtBeginConditionalFormatting14);

        if(ptr->m_arCFRULE14.empty())
            return;

        for(auto &pCFRULE14: ptr->m_arCFRULE14)
            m_arrItems.push_back(new CConditionalFormattingRule(pCFRULE14));
    }
    IsExtended();
}

XLS::BaseObjectPtr CConditionalFormatting::toBin()
{
    XLS::BaseObjectPtr objectPtr;
    if(m_arrItems.empty())
    {
        return objectPtr;
    }
    auto ptr(new XLSB::CONDITIONALFORMATTING);
    objectPtr = XLS::BaseObjectPtr{ptr};
    XLS::CellRef formatingfirstCell;

    auto conditionPtr(new XLSB::BeginConditionalFormatting);
    ptr->m_BrtBeginConditionalFormatting = XLS::BaseObjectPtr{conditionPtr};
    conditionPtr->ccf = m_arrItems.size();
    if(m_oSqRef.IsInit())
    {
        conditionPtr->sqrfx.strValue = m_oSqRef.get();
    }
    else
        conditionPtr->sqrfx.crfx = 0;
    if(m_oPivot.IsInit())
        conditionPtr->fPivot = m_oPivot->GetValue();
    else
        conditionPtr->fPivot = false;
    formatingfirstCell = conditionPtr->sqrfx.getLocationFirstCell();

    for(auto i: m_arrItems)
    {
        ptr->m_arCFRULE.push_back(i->toBin(formatingfirstCell));
    }
    return objectPtr;
}
void CConditionalFormatting::toBin(XLS::StreamCacheWriterPtr& writer)
{
    XLS::CellRef formatingfirstCell;
    {
        auto begin = writer->getNextRecord(XLSB::rt_BeginConditionalFormatting);
        _UINT32 ccf = m_arrItems.size();
        *begin <<ccf;
        ccf = 0;
        if(m_oPivot.IsInit())
            ccf = m_oPivot->GetValue();
        *begin <<ccf;
        XLSB::UncheckedSqRfX sqrfx;
        if(m_oSqRef.IsInit())
            sqrfx.strValue = m_oSqRef.get();
        formatingfirstCell = sqrfx.getLocationFirstCell();
        *begin <<sqrfx;
        writer->storeNextRecord(begin);
    }
     for(auto i: m_arrItems)
     {
        if(m_bIsExtended)
            i->m_oExtId = L"{" + XmlUtils::GenerateGuid() + L"}";
        i->toBin(writer, formatingfirstCell);
     }
    {
        auto end = writer->getNextRecord(XLSB::rt_EndConditionalFormatting);
        writer->storeNextRecord(end);
    }
}

XLS::BaseObjectPtr CConditionalFormatting::toBin14()
{
    auto ptr(new XLSB::CONDITIONALFORMATTING14);
    auto ptr1(new XLSB::BeginConditionalFormatting14);
    ptr->m_BrtBeginConditionalFormatting14 = XLS::BaseObjectPtr{ptr1};
    XLS::BaseObjectPtr objectPtr = XLS::BaseObjectPtr{ptr};

    ptr1->FRTheader.fFormula = false;
    ptr1->FRTheader.fRelID = false;
    ptr1->FRTheader.fRef = false;
    ptr1->FRTheader.fSqref = true;
    XLS::CellRef formatingfirstCell;
    if(m_oSqRef.IsInit())
    {
        XLSB::FRTSqref sqref;
        sqref.sqrfx.strValue = m_oSqRef.get();
        ptr1->FRTheader.rgSqrefs.array.push_back(sqref);
        formatingfirstCell = sqref.sqrfx.getLocationFirstCell();
    }
    ptr1->ccf = m_arrItems.size();
    if(m_oPivot.IsInit())
        ptr1->fPivot = m_oPivot->GetValue();
    else
        ptr1->fPivot = false;

    for(auto i: m_arrItems)
    {
        ptr->m_arCFRULE14.push_back(i->toBin14(formatingfirstCell));
    }
    return objectPtr;
}

bool CConditionalFormatting::IsUsage()
{
    for ( size_t i = 0; i < m_arrItems.size(); ++i)
    {
        if ( (m_arrItems[i]) && (m_arrItems[i]->bUsage)) return true;
    }
	return false;
}
void CConditionalFormatting::ReadAttributes(XmlUtils::CXmlLiteReader& oReader)
{
	WritingElement_ReadAttributes_Start(oReader)

	WritingElement_ReadAttributes_Read_if		(oReader, L"sqref"	, m_oSqRef)
	WritingElement_ReadAttributes_Read_else_if	(oReader, L"pivot"	, m_oPivot)

	WritingElement_ReadAttributes_End(oReader)
}
void CConditionalFormatting::ReadAttributes(XLS::BaseObjectPtr& obj)
{
    if(obj->get_type() == XLS::typeBeginConditionalFormatting)
    {
        auto ptr = static_cast<XLSB::BeginConditionalFormatting*>(obj.get());
        if(ptr != nullptr)
        {
            m_oSqRef = ptr->sqrfx.strValue;
            m_oPivot = (bool)ptr->fPivot;
        }
    }
    else if(obj->get_type() == XLS::typeBeginConditionalFormatting14)
    {
        auto ptr = static_cast<XLSB::BeginConditionalFormatting14*>(obj.get());
        if(ptr != nullptr)
        {
            if(ptr->FRTheader.fSqref && !ptr->FRTheader.rgSqrefs.array.empty())
                m_oSqRef = ptr->FRTheader.rgSqrefs.array[0].sqrfx.strValue;
            m_oPivot = (bool)ptr->fPivot;
        }
    }

}

} //Spreadsheet
} // namespace OOX
