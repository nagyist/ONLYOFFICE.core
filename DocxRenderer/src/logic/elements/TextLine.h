#pragma once
#include "ContText.h"
#include "BaseItem.h"

namespace NSDocxRenderer
{
	class CTextLine : public CBaseItem, public IOoxmlItem
	{
	public:
		enum AssumedTextAlignmentType
		{
			atatUnknown,
			atatByLeftEdge,
			atatByCenter,
			atatByRightEdge,
			atatByWidth
		};

		std::vector<std::shared_ptr<CContText>> m_arConts;

		AssumedTextAlignmentType m_eAlignmentType{atatUnknown};
		eVertAlignType m_eVertAlignType          {eVertAlignType::vatUnknown};

		std::shared_ptr<CTextLine> m_pLine;
		std::shared_ptr<CShape>  m_pDominantShape {nullptr};

		UINT m_iNumDuplicates {0};

		double m_dTopWithMaxAscent{0};
		double m_dBotWithMaxDescent{0};

		double m_dFirstWordWidth{0.0};

		bool m_bIsPossibleVerSplit = false;

	public:
		CTextLine() = default;
		virtual ~CTextLine();
		virtual void Clear();
		virtual void ToXml(NSStringUtils::CStringBuilder& oWriter) const override final;
		virtual void ToXmlPptx(NSStringUtils::CStringBuilder& oWriter) const override final;
		virtual void ToBin(NSWasm::CData& oWriter) const override final;
		virtual void RecalcWithNewItem(const CContText* pCont);
		virtual eVerticalCrossingType GetVerticalCrossingType(const CTextLine* pLine) const noexcept;

		void AddCont(const std::shared_ptr<CContText>& pCont);
		void AddConts(const std::vector<std::shared_ptr<CContText>>& arConts);
		void MergeConts();
		void CalcFirstWordWidth();
		void RecalcSizes();
		void SetVertAlignType(const eVertAlignType& oType);

		bool IsShadingPresent(const CTextLine* pLine) const noexcept;
		bool IsCanBeDeleted() const;

		double GetLeftNoEnum() const noexcept;

		size_t GetLength() const;
		void GetNextSym(size_t& nContPos, size_t& nSymPos) const noexcept;
	};
}
