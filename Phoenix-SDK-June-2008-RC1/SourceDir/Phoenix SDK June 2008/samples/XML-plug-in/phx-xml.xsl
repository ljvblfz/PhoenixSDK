<?xml version="1.0" encoding="UTF-8" ?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

	<xsl:template match="/">
		<HTML>
		   <head>
		   <title>
		      FuncUnit:  <xsl:value-of select="root/func-unit/@name" />
		      : <xsl:value-of select="root/func-unit/@ir-state" />
		      from <xsl:value-of select="root/func-unit/@phase" />
		   </title>
		   <style type="text/css">
		      .op {font-size: 9pt;}
		      .var-opnd {font-size: 9pt; color: #0000FF}
		      .mem-opnd {font-size: 9pt; color: #000088}
		      .imm-opnd {font-size: 9pt; color: #FF0000}
		      .label-opnd {font-size: 9pt}
		      .func-opnd {font-size: 9pt}
		      .modref-opnd {font-size: 9pt; color: #888888}
		      .line {font-size: 9pt}
		      .typeOn {display: inline}
		      .typeOff {display: none}
		   </style>
		   
		   <script language="JScript">
		   </script>
		   
		   </head>
			<BODY STYLE="font-family:Arial, helvetica, sans-serif; font-size:9pt;
				background-color:#FFFFFF">
				<xsl:apply-templates select="root" />
			</BODY>
		</HTML>
	</xsl:template>
	
   <!-- ================================================   root   -->

   <xsl:template match="root">
	   <xsl:apply-templates select="phases" />
	   <xsl:apply-templates select="func-unit" />
	</xsl:template>
	
   <!-- ================================================   func-unit   -->
	
	<xsl:template match="func-unit">
	   <h2>
	      FuncUnit:<br />
	      <xsl:value-of select="@name"/>
	       (<xsl:value-of select="@ir-state" />)
		    (<xsl:value-of select="@source" />)
	   </h2>
	   <!--  these simply output the sample controls' values - - >
       <p/>
       -d2 XMLint:<xsl:value-of select="@XMLint" />
       <p/>
       -d2 XMLstr:<xsl:value-of select="@XMLstr" />
       <p/>
       -d2 XMLset:<xsl:value-of select="@XMLset" />
       < ! - - -->
       
	   <xsl:apply-templates select="blocks" />
	</xsl:template>

   <!-- ================================================   phases   -->
	
	<xsl:template match="phases">
	   <h2>Phases</h2>
	   <ul>
	   <xsl:apply-templates />
	   </ul>
	   <hr />
	</xsl:template>
	
	<xsl:template match="subphase-list">
	   <li> <xsl:value-of select="@name" /> </li>
	   <ul>
	   <xsl:apply-templates />
	   </ul>
	</xsl:template>
	
	<xsl:template match="phase">
	   <li> <xsl:value-of select="@name" /> </li>
	</xsl:template>
	
   <!-- ================================================   blocks    -->
	
	<xsl:template match="blocks">
	   <h2>IR by basic blocks</h2>
	   <xsl:apply-templates />
	</xsl:template>
	
	<xsl:template match="block">
	   <hr />
	   <xsl:apply-templates />
	</xsl:template>
	
   <!-- ================================================   instr    -->
	
	<xsl:template match="instr">
	   <table rules="cols" border="0">
	   <tr>
	      <td width="120pt">
	         <xsl:apply-templates select="./dst-opnd-list" />
	      </td>
	      <td width="100pt" class="op" align="left">
   	      <xsl:value-of select="@op" />
   	      <xsl:if test="@cc != ''">
   	         <sub><i><xsl:value-of select="@cc" /></i></sub>
   	      </xsl:if>
         </td>
   	   <td width="400pt">
	         <xsl:apply-templates select="./src-opnd-list" />
	      </td>
	      <td class="line">
   	      #<xsl:value-of select="@line" />
	      </td>
	   </tr>
	   </table>
	</xsl:template>
	
	<!-- ==================================================   operands   -->
	
	<xsl:template match="src-opnd-list | dst-opnd-list">
	   <xsl:apply-templates />
	</xsl:template>
	
	<xsl:template match="var-opnd">
	   <span class="var-opnd" onMouseOver="this.Child('srcOpndType').style.color='#000000'"
	      onMouseOut="this.srcOpndType.style.color='#0000FF'">
	      <xsl:choose>
	         <xsl:when test="@is-addr = 'true'">
      	      <b>&amp;</b> <xsl:value-of select="substring(@display,2)" />
      	   </xsl:when>
      	   <xsl:otherwise>
      	      <xsl:value-of select="@display" />
      	   </xsl:otherwise>
      	</xsl:choose>
      	<span id="srcOpndType" class="typeOn">
	      <sub><i><xsl:value-of select="@type" /></i></sub>
	      </span>
	   </span>
	   ,
	</xsl:template>
	
	<xsl:template match="mem-opnd">
	   <span class="mem-opnd">
	      <xsl:choose>
	         <xsl:when test="@is-addr = 'true'">
      	      <b>&amp;</b> <xsl:value-of select="substring(@display,2)" />
      	   </xsl:when>
      	   <xsl:otherwise>
      	      <xsl:value-of select="@display" />
      	   </xsl:otherwise>
      	</xsl:choose>
      	<span id="dstOpndType" class="typeOn">
	      <sub><i><xsl:value-of select="@type" /></i></sub>
	      </span>
	   </span>
	   ,
	</xsl:template>
	
	<xsl:template match="imm-opnd">
	   <span class="imm-opnd">
	      <xsl:value-of select="@*" />
	   </span>
	   ,
	</xsl:template>
	
	<xsl:template match="func-opnd">
	   <span class="func-opnd">
	      <xsl:choose>
   	      <xsl:when test="@is-def = 'true'">
	            <!--  the target filename is the same as created by the XML dump phase -->
   	         <a href="./[{../../../../../@phase}]{@sym}.xml"><xsl:value-of select="@sym" /></a>
      	   </xsl:when>
      	   <xsl:otherwise>
               <xsl:value-of select="@sym" />
      	   </xsl:otherwise>
      	</xsl:choose>
	   </span>
	   ,
	</xsl:template>
	
	<xsl:template match="label-opnd">
	   <span class="label-opnd">
	      <xsl:choose>
	         <xsl:when test="name(..) = 'src-opnd-list'">
	            <a href="#{@name}"><xsl:value-of select="@name" /></a>
	         </xsl:when>
	         <xsl:otherwise>
	            <a name="{@name}"><xsl:value-of select="@name" /></a>
	         </xsl:otherwise>
	      </xsl:choose>
	      <!--
	      <sub><i><xsl:value-of select="@kind" /></i></sub>
	      -->
	   </span>
	   ,
	</xsl:template>
	
	<xsl:template match="modref-opnd">
	   <span class="modref-opnd">
	      <xsl:value-of select="@alias-tag" />
	   </span>
	   ,
	</xsl:template>
	
</xsl:stylesheet>


