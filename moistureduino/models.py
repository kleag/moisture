from django.db import models


class Entry(models.Model):
    created = models.DateTimeField(auto_now_add=True)
    kind = models.CharField(max_length=100, blank=True, default='')
    value = models.CharField(max_length=100, blank=True, default='')
    owner = models.ForeignKey('auth.User', related_name='entries',
                              on_delete=models.CASCADE)
    # highlighted = models.TextField()

    class Meta:
        ordering = ['created']

    def save(self, *args, **kwargs):
        """
        """
        # lexer = get_lexer_by_name(self.language)
        # linenos = 'table' if self.linenos else False
        # options = {'title': self.title} if self.title else {}
        # formatter = HtmlFormatter(style=self.style, linenos=linenos,
                                  # full=True, **options)
        # self.highlighted = highlight(self.code, lexer, formatter)
        super(Entry, self).save(*args, **kwargs)
