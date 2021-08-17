<?xml version="1.0"?>
<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:template match="/">
    <html>
      <body>
        <h2>Test Results</h2>
        <table border="1">
          <tr bgcolor="#5588ff">
            <th>Test</th>
            <th>Result</th>
            <th>Standard Output</th>
            <th>Error Output</th>
            <th>Error Message</th>
          </tr>
          <xsl:for-each select="test_results/test">
            <tr>
              <td>
                <xsl:value-of select="test_script"/>
              </td>
              <xsl:choose>
                <xsl:when test="result = 'fail'">
                  <td bgcolor="#ff3333">
                    <xsl:value-of select="result"/>
                  </td>
                </xsl:when>
                <xsl:otherwise>
                  <td>
                    <xsl:value-of select="result"/>
                  </td>
                </xsl:otherwise>
              </xsl:choose>
              <td>
                <xsl:value-of select="stdout_output"/>
              </td>
              <td>
                <xsl:value-of select="stderr_output"/>
              </td>
              <td>
                <xsl:value-of select="error_message"/>
              </td>
            </tr>
          </xsl:for-each>
        </table>
      </body>
    </html>
  </xsl:template>

</xsl:stylesheet>